/*************************************************************************

      Graphics Research Group - Department of Computer Science
                  University of Sheffield, UK
                      Michael Meredith
                     Copyright (c) 2000
                    All Rights Reserved.


  Permission is hereby granted, free of charge, to use this software 
  and its documentation without restriction, including without limitation 
  the rights to use, copy, modify, merge and publish, subject to the following 
  conditions:

   1. The code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Any modifications must be clearly marked as such.

   3. Original authors' names are not deleted.

   4. The authors' names are not used to endorse or promote products
      derived from this software without specific prior written
      permission from the author.

   5. This software is used in a research, non-profit capacity only and not 
      for commercial or profit applications unless specific prior written
      permission from the author is given.


  THE UNIVERSITY OF SHEFFIELD AND THE CONTRIBUTORS TO THIS WORK
  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
  SHALL THE UNIVERSITY OF SHEFFIELD NOR THE CONTRIBUTORS BE LIABLE
  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
  THIS SOFTWARE.

*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "HTRFormat.h"


bool HTRFormat::ImportData(const char *filename)
{
  int read, i, j, where;
  int pos[8];           // Used to determine the position of the next char to write
  char line[8][40];     // Used to store the attribute and the corresponding value
  char buffer[4097];
  int section = 0;      // Indicates which section is currently being processed
  NODE *tnode;          // Used when setting up the base positions and frames

  header->callib = 1.0f;
  header->scalefactor = 1.0f;

  FILE *file = fopen(filename, "rb");
  if (file)
  {
    // Process the "Header" section of the file
    read = fread(buffer,1,4096,file);
    buffer[read] = '\0';
    i = strstrEx(buffer, "[HEADER]");
    i+=strstrEx(buffer + i, char(10));
    while (buffer[++i] < 32);

    where = pos[0] = pos[1] = pos[2] = pos[3] = pos[4] = pos[5] = pos[6] = pos[7] = 0;
    // Process each line in the header
    while (read)
    {
      while (i<read)
      {
        if (buffer[i] =='#' || buffer[i] == char(10))
        {
          // Process line
          line[1][pos[1]] = line[0][pos[0]] = '\0';
          if (line[0][0]=='[')
          {
            if (++section==2)
            {
              // Body structure has been read and ready to process the base positions
              // So assign the GLOBAL node to the root pointer
              root = 0;
              for (int j=0; j<currentnode && !root; ++j)
              {
                if (strcompEx(nodelist[j]->name, "GLOBAL"))
                {
                  root = nodelist[j]->children[0];
                  NODE **temp = (NODE**) malloc(sizeof(NODE*) * header->noofsegments);
                  int m;
                  for (m=0; m < j; ++m)
                    temp[m] = nodelist[m];
                  for (m=j+1; m<=header->noofsegments; ++m)
                    temp[m-1] = nodelist[m];
                  
                  free(nodelist[j]->name);
                  free(nodelist[j]->children);
                  for (m=0; m<header->noofframes; ++m)
                  {
                    free(nodelist[j]->froset[m]);
                    free(nodelist[j]->freuler[m]);
                  }
                  free(nodelist[j]->froset);
                  free(nodelist[j]->freuler);
                  free(nodelist[j]->scale);

                  free(nodelist);
                  nodelist = temp;
                  --currentnode;
                }
              }
            }
            if (section>2)
            {
              char temp[40];
              j = 1;
              while((temp[j-1]=line[0][j]) && line[0][++j]!=']');
              temp[j-1] = '\0';
              tnode = 0;
              for (j=0; j<currentnode && !tnode; ++j)
              {
                if (!strcmp(nodelist[j]->name, temp))
                  tnode = nodelist[j];
              }
              if (!tnode)
              {
                if (strcmp(temp, "EndOfFile"))
                {
                  strcpy(error, "Unknown node has been encountered while processing the frames");
                  fclose(file);
                  return false;
                }
                else
                {
                  fclose(file);
                  return true;
                }
              }
            }
          }
          else if (line[0][0] && line[1][0])
          {
            if (!section)
            {
              if (!ProcessHeader(line, pos))
              {
                fclose(file);
                return false;
              }
            }
            else if (section==1)
            {
              if (!ProcessSegments(line, pos))
              {
                fclose(file);
                return false;
              }
            }
            else if (section==2)
            {
              // Setup the base positions for the segment
              line[7][pos[7]] = line[6][pos[6]] = line[5][pos[5]] = line[4][pos[4]] = line[3][pos[3]] = line[2][pos[2]] = line[1][pos[1]] = '\0';
              tnode = 0;
              for (j=0; j<currentnode && !tnode; ++j)
                if (!strcmp(nodelist[j]->name, line[0]))
                  tnode = nodelist[j];
              SetupColour(tnode);
              SetupOffset(tnode, (float) atof(line[1]) * header->callib, (float) atof(line[2]) * header->callib, (float) atof(line[3]) * header->callib);
              tnode->euler[xpos] = (float) atof(line[4]);
              tnode->euler[ypos] = (float) atof(line[5]);
              tnode->euler[zpos] = (float) atof(line[6]);
              tnode->length[0] = 0.0f;
              tnode->length[1] = (float) atof(line[7]) * header->callib;
              tnode->length[2] = 0.0f;
              tnode->DOFs = XROT | YROT | ZROT | XTRA | YTRA | ZTRA;
            }
            else if (section>2)
            {
              // Process the frame information for tnode
              line[7][pos[7]] = line[6][pos[6]] = line[5][pos[5]] = line[4][pos[4]] = line[3][pos[3]] = line[2][pos[2]] = line[1][pos[1]] = '\0';
              long frame = atol(line[0]) - 1;
              tnode->froset[frame][0] = (float) atof(line[1]) * header->callib;
              tnode->froset[frame][1] = (float) atof(line[2]) * header->callib;
              tnode->froset[frame][2] = (float) atof(line[3]) * header->callib;
              tnode->freuler[frame][xpos] = (float) atof(line[4]);
              tnode->freuler[frame][ypos] = (float) atof(line[5]);
              tnode->freuler[frame][zpos] = (float) atof(line[6]);
              tnode->scale[frame] = (float) atof(line[7]);
            }
          }
          
          // Move onto the next line and clear current line information
          j=strstrEx(buffer + i, char(10));
          if (j==-1)
          {
            if (buffer[4095]!=10)
            {
              read = fread(buffer, 1, 4096, file);
              i = strstrEx(buffer, char(10));
            }
            else
            {
              read = fread(buffer, 1, 4096, file);
              i=0;
            }
            buffer[4096] = '\0';
          }
          else
            i+=j;
          where = pos[0] = pos[1] = pos[2] = pos[3] = pos[4] = pos[5] = pos[6] = pos[7] = 0;
        }

        if (buffer[i] > 44 && buffer[i] < 123)
          line[where][pos[where]++] = buffer[i++];
        else if ((buffer[i]==32 || buffer[i]==9) && pos[where]>0)// && (where==0 || section>1))
        {
          ++where;
          ++i;
        }
        else
          ++i;
      }
      read = fread(buffer, 1, 4096, file);
      buffer[4096] = '\0';
      i=0;
    }
    fclose(file);
    return true;
  }
  else
  {
    strcpy(error, "Cannot Open File");
    return false;
  }
}

bool HTRFormat::ProcessHeader(char line[2][40], int pos[2])
{
  if (strcompEx(line[0], "FILETYPE"))
  {
    if (!strcompEx(line[1], "HTR"))
    {
      strcpy(error, "Filetype is not HTR");
      return false;
    }
  }
  else if (strcompEx(line[0], "DATATYPE"))
  {
    // Datatype describes the order of transformation composition of translation, rotation
    // and scale values.  Almost always "HTRS" - Translation, Rotation, Scale
    if (!strcompEx(line[1], "HTRS"))
    {
      strcpy(error, strcat("Error: Incompatible transformation composition: ",line[1]));
    }
  }
  else if (strcompEx(line[0], "FILEVERSION"))
  {
    if (atoi(line[1])!=1)
    {
      strcpy(error, "File version not not recognised");
      return false;
    }
  }
  else if (strcompEx(line[0], "NUMSEGMENTS"))
  {
    header->noofsegments = atoi(line[1]);
    free(nodelist);
    nodelist = (NODE**) malloc(sizeof(NODE*) * (header->noofsegments + 1));
    currentnode = 0;
  }
  else if (strcompEx(line[0], "NUMFRAMES"))
    header->noofframes = atol(line[1]);
  else if (strcompEx(line[0], "DATAFRAMERATE"))
    header->datarate = atoi(line[1]);
  else if (strcompEx(line[0], "EULERROTATIONORDER"))
    // Setup the euler rotation angle
    for (int i=0; i<3; ++i)
    {
      switch (line[1][i]&0xdf)
      {
        case 'Z':
          header->euler[i][1] = header->euler[i][0] = 0;
          header->euler[i][2] = 1;
          zpos = i;
          break;
        case 'Y':
          header->euler[i][2] = header->euler[i][0] = 0;
          header->euler[2-i][1] = 1;
          ypos = i;
          break;
        case 'X':
          header->euler[i][2] = header->euler[i][1] = 0;
          header->euler[i][0] = 1;
          xpos = i;
          break;
      }
    }
  else if (strcompEx(line[0], "CALIBRATIONUNITS"))
    if (strcompEx(line[1], "MM"))
      header->callib = 1.0f / 1000.0f;
    else if (strcompEx(line[1], "DM"))
      header->callib = 1.0f / 10.0f;
    else if (strcompEx(line[1], "CM"))
      header->callib = 1.0f / 100.0f;
    else
      header->callib = 1.0f;
  else if (strcompEx(line[0], "ROTATIONUNITS"))
    if (strcompEx(line[1], "DEGREES"))
      header->degrees = true;
    else
      header->degrees = false;
  else if (strcompEx(line[0], "SCALEFACTOR"))
    header->scalefactor = (float) atof(line[1]);

  return true;
}

bool HTRFormat::ProcessSegments(char line[2][40], int pos[2])
{
  // This method simply reads in each segment and makes sure that a node exists for it, linking the nodes as it goes
  // Create an array of nodes that will hold the names of each segment which will be linked
  NODE *parent = 0, *child = 0, **temp;   // temp used to add new children to parent nodes
  int i;

  // line[0] = Child     line[1] = Parent
  // Get the position within the segment structure of each node - if there is no current entry then create a new on at the end
  for (i=0; i<currentnode; ++i)
  {
    if (!strcmp(nodelist[i]->name, line[0]))
      child = nodelist[i];
    if (!strcmp(nodelist[i]->name, line[1]))
      parent = nodelist[i];
  }
 
  if (child && child==parent)
  {
    strcpy(error, "Error: A cyclic child-parent relationship has been detected in the Segment section of the file");
    return false;
  }

  if (!child)
  {
    nodelist[currentnode] = (NODE*) malloc(sizeof(NODE));
    nodelist[currentnode]->name = (char*) malloc(sizeof(char) * (strlen(line[0]) + 1));
    strcpy(nodelist[currentnode]->name, line[0]);
    SetupChildren(nodelist[currentnode], 0);
    SetupFrames(nodelist[currentnode], header->noofframes);
    child = nodelist[0];
    child = nodelist[currentnode++];
  }

  if (!parent)
  {
    nodelist[currentnode] = (NODE*) malloc(sizeof(NODE));
    nodelist[currentnode]->name = (char*) malloc(sizeof(char) * (strlen(line[1]) + 1));
    strcpy(nodelist[currentnode]->name, line[1]);
    SetupChildren(nodelist[currentnode], 0);
    SetupFrames(nodelist[currentnode], header->noofframes);
    parent = nodelist[currentnode++];
  }

  if (parent->children)
  {
    // Parent already has children
    temp = (NODE**) malloc(sizeof(NODE*) * parent->noofchildren);
    for (i=0; i<parent->noofchildren; ++i)
      temp[i] = parent->children[i];
    free(parent->children);
    parent->children = (NODE**) malloc(sizeof(NODE*) * ++parent->noofchildren);
    for (i=0; i<parent->noofchildren; ++i)
      parent->children[i] = temp[i];
    free(temp);
    parent->children[parent->noofchildren-1] = child;
  }
  else
  {
    // Parent does not have any children
    SetupChildren(parent, ++parent->noofchildren);
    parent->children[0] = child;
  }

  if (currentnode>header->noofsegments+1)
  {
    strcpy(error, "There are more segments in the datafile than specified in the header section of the file.\nCannot open data file");
    return false;
  }
  else
    return true;
}
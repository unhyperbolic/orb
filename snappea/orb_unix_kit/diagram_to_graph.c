#include "diagram_to_graph.h"

#include <stdlib.h>

static int cmp(double d1, double d2)
{
    if (d1 < d2) {
	return -1;
    }
    if (d1 > d2) {
	return +1;
    }
    return 0;
}

static int ed_more(const void *d1, const void *d2)
{
    // return ed1->angle > ed2->angle;

  
    DiagramEndData * ed1 = *(DiagramEndData **)d1;
    DiagramEndData * ed2 = *(DiagramEndData **)d2;

    return cmp(ed2->angle, ed1->angle);
}

static int crossing_less(const void *d1, const void *d2)
{
    DiagramCrossing *c1 = *(DiagramCrossing **)d1;
    DiagramCrossing *c2 = *(DiagramCrossing **)d2;
    
    if (c1->over == c2->over)
    {
	return cmp(c1->position_on_overstrand, c2->position_on_overstrand);
    }

    if (c1->over == c2->under)
    {
	return cmp(c1->position_on_overstrand, c2->position_on_understrand);
    }

    if (c1->under == c2->over)
    {
	return cmp(c1->position_on_understrand, c2->position_on_overstrand);
    }

    if (c1->under == c2->under)
    {
	return cmp(c1->position_on_understrand, c2->position_on_understrand);
    }

    return 0;
}

void remove_meeting( Graph *graph, int index )
{
    if ( index>= graph->num_meetings)
    {
	return;
    }

    GraphMeeting *old_array = graph->meeting;
    GraphMeeting *new_array = NEW_ARRAY(graph->num_meetings-1, GraphMeeting );

    for (int i = 0, j=0; i< graph->num_meetings-1; i++ )
    {
        if (i == index)
	{
	    j++;
	}

        new_array[i].type = old_array[i+j].type;
	new_array[i].handedness = old_array[i+j].handedness;
        new_array[i].num_strands = old_array[i+j].num_strands;
        new_array[i].label = old_array[i+j].label;
        new_array[i].strand = old_array[i+j].strand;
        new_array[i].component = old_array[i+j].component;
        new_array[i].neighbor = old_array[i+j].neighbor;
    }

    my_free(old_array);
    graph->meeting = new_array;
    graph->num_meetings--;

    for(int i = 0; i < graph->num_meetings; i++)
    {
	for(int j=0; j < graph->meeting[i].num_strands; j++)
	{
	    if (graph->meeting[i].neighbor[j] >= index)
	    {
		graph->meeting[i].neighbor[j]--;
	    }
	}
    }
}

Graph * diagram_to_graph(Diagram * diagram)
{
    int num_meetings = 0;

    diagram_get_crossing_signs(diagram);
    diagram_ed_angles(diagram);
    assign_diagram_crossings_to_edges(diagram);
    prepare_diagram_components_for_output(diagram);

    for(int i = 0; i < diagram->num_vertices; i++)
    {
        diagram->vertices[i]->vertex_id = num_meetings++;
	qsort(diagram->vertices[i]->incident_end_data,
	      diagram->vertices[i]->num_incident_end_data,
	      sizeof(DiagramEndData*),
	      &ed_more);
    }

    for(int i = 0; i < diagram->num_crossings; i++)
    {
        diagram->crossings[i]->crossing_id = num_meetings++;
    }

    for(int i = 0; i < diagram->num_edges; i++) {
        qsort(diagram->edges[i]->crossings,
	      diagram->edges[i]->num_crossings,
	      sizeof(DiagramCrossing*),
	      &crossing_less);
    }

    Graph *graph = NEW_STRUCT(Graph);
    graph->num_meetings = num_meetings;
    graph->num_components = diagram->num_links;
    graph->num_free_loops = 0;
    graph->meeting = NEW_ARRAY( num_meetings, GraphMeeting );
   
    for(int i = 0; i < diagram->num_vertices; i++)
    {
        GraphMeeting *meeting = &graph->meeting[i];
	DiagramVertex *vertex = diagram->vertices[i];

	meeting->type = Inter;
	meeting->num_strands = vertex->num_incident_end_data;
	meeting->strand = NEW_ARRAY(meeting->num_strands,int);
	meeting->neighbor = NEW_ARRAY(meeting->num_strands,int);
	meeting->component = NEW_ARRAY(meeting->num_strands,int);
	meeting->label = NEW_ARRAY(meeting->num_strands,int);
	meeting->tet = NULL;
	meeting->handedness = 0;

	for(int j = 0; j < meeting->num_strands; j++)
        {
	    DiagramEndData *ed = vertex->incident_end_data[j];
	    DiagramEdge *e = ed->edge;
	    meeting->label[j] = (ed->singular) ? e->arc_id : -1;

	    meeting->component[j] = diagram->vertices[i]->link_id;

	    if (ed->type == diagramBegin)
	    {
	        if (e->num_crossings == 0)
		{
		    meeting->strand[j] = diagram_get_strand(e, e->vertex[diagramEnd]);
		    meeting->neighbor[j] = e->vertex[diagramEnd]->vertex_id;
		}
		else
		{
  		    DiagramCrossing *c = e->crossings[0];
		    meeting->neighbor[j] = c->crossing_id;

		    if (c->over == e)
		    {
		        meeting->strand[j] = 0;
		    }
		    else
		    {
		        meeting->strand[j] = 1;
		    } 
                }
	    }
	    else
	    {
	        if (e->num_crossings == 0)
		{
		    meeting->strand[j] = diagram_get_strand(e, e->vertex[diagramBegin]);
		    meeting->neighbor[j] = e->vertex[diagramBegin]->vertex_id;
		}
		else
		{
		    DiagramCrossing *c = e->crossings[e->num_crossings-1];
		    meeting->neighbor[j] = c->crossing_id;

		    if (c->over == e)
		    {
		        meeting->strand[j] = 2;
		    }
		    else
		    {
		        meeting->strand[j] = 3;
		    }

		}
	    }
	}
    }

    for(int i=0;i<diagram->num_crossings;i++)
    {
          GraphMeeting *meeting = &graph->meeting[diagram->num_vertices+i];
          DiagramCrossing *c = diagram->crossings[i], *other;
          DiagramEdge *e;
          int j;

          meeting->type = Cross;
          meeting->num_strands = 4;
          meeting->strand = NEW_ARRAY( 4, int );
          meeting->neighbor = NEW_ARRAY( 4, int ); 
          meeting->component = NEW_ARRAY( 4, int ); 
          meeting->label = NEW_ARRAY( 4, int ); 
          meeting->tet = NULL;
	  meeting->handedness = c->crossing_sign;

          for (j=0;j<4;j++)
                meeting->label[j] = -1;

          e = c->over;

          meeting->component[0] = e->link_id;
          meeting->component[2] = e->link_id;

          if ( (other = get_prev_crossing( e, c)) == NULL)
          {
                meeting->strand[0] = diagram_get_strand( e, e->vertex[diagramBegin] );
                meeting->neighbor[0] = e->vertex[diagramBegin]->vertex_id;
          }
          else
          {
                meeting->neighbor[0] = other->crossing_id;
                meeting->strand[0] = (other->over==e) ? 2 : 3;
          }

          if ( (other = get_next_crossing( e, c)) == NULL)
          {
                meeting->strand[2] = diagram_get_strand( e, e->vertex[diagramEnd] );
                meeting->neighbor[2] = e->vertex[diagramEnd]->vertex_id;
          }
          else
          {
                meeting->neighbor[2] = other->crossing_id;
                meeting->strand[2] = (other->over==e) ? 0 : 1;
          }

          e = c->under;

          meeting->component[1] = e->link_id;
          meeting->component[3] = e->link_id;

          if ( (other = get_prev_crossing( e, c)) == NULL)
          {
                meeting->strand[1] = diagram_get_strand( e, e->vertex[diagramBegin] );
                meeting->neighbor[1] = e->vertex[diagramBegin]->vertex_id;
          }
          else
          {
                meeting->neighbor[1] = other->crossing_id;
                meeting->strand[1] = (other->over==e) ? 2 : 3;
          }

          if ( (other = get_next_crossing( e, c)) == NULL)
          {
                meeting->strand[3] = diagram_get_strand( e, e->vertex[diagramEnd] );
                meeting->neighbor[3] = e->vertex[diagramEnd]->vertex_id;
          }
          else
          {
                meeting->neighbor[3] = other->crossing_id;
                meeting->strand[3] = (other->over==e) ? 0 : 1;
          }

    }

    for(int i = 0; i < diagram->num_crossings; i++)
    if (diagram->crossings[i]->crossing_sign == 1)
    {
          int temp_strand , temp_neighbor;
          GraphMeeting *meeting, *nbr1, *nbr3;
        
          meeting = &graph->meeting[diagram->crossings[i]->crossing_id];

          nbr1 = &graph->meeting[meeting->neighbor[1]];
          nbr3 = &graph->meeting[meeting->neighbor[3]];

          nbr1->strand[ meeting->strand[1] ] = 3;
          nbr3->strand[ meeting->strand[3] ] = 1;

          temp_strand = meeting->strand[3];
          temp_neighbor = meeting->neighbor[3];

          meeting->strand[3] = meeting->strand[1];
          meeting->neighbor[3] = meeting->neighbor[1];

          meeting->strand[1] = temp_strand;
          meeting->neighbor[1] = temp_neighbor;
    }

    /*
    for(int i = 0; i < diagram->num_edges; i++)
    {
        diagram->edges[i]->crossings.clear();
    }
    */

    int i = 0;
    while (i < graph->num_meetings)
    {
            GraphMeeting *meeting = &graph->meeting[i];
            if (meeting->num_strands == 2
                && meeting->label[0] == -1 && meeting->label[1] == -1 )
            {
                  GraphMeeting *nbr0, *nbr1;

                  nbr0 = &graph->meeting[meeting->neighbor[0]];
                  nbr1 = &graph->meeting[meeting->neighbor[1]];

                  if (nbr0 == meeting && nbr1 == meeting)
                  {
			/* let's put a twist in this component */

                        int component = meeting->component[0];

			my_free( meeting->component );
			my_free( meeting->label );
			my_free( meeting->strand );
			my_free( meeting->neighbor );

			meeting->type		= Cross;
			meeting->label 		= NEW_ARRAY( 4, int );
			meeting->component	= NEW_ARRAY( 4, int );
			meeting->neighbor	= NEW_ARRAY( 4, int );
			meeting->strand		= NEW_ARRAY( 4, int );

			meeting->num_strands = 4;

        		for ( int j=0; j< 4; j++)
        		{
                		meeting->label[j] = -1;
				meeting->component[j] = component;
                		meeting->neighbor[j] = i;
        		}	

        		meeting->strand[0] = 3;
        		meeting->strand[1] = 2;
        		meeting->strand[2] = 1;
        		meeting->strand[3] = 0;
			meeting->handedness = 1;
			i++;
                  }
                  else
                  {
                        nbr0->strand[meeting->strand[0]] = meeting->strand[1];
                        nbr1->strand[meeting->strand[1]] = meeting->strand[0];

                        nbr0->neighbor[meeting->strand[0]] = meeting->neighbor[1];
                        nbr1->neighbor[meeting->strand[1]] = meeting->neighbor[0]; 
			my_free(meeting->strand);
			my_free(meeting->neighbor);
			my_free(meeting->component);
			my_free(meeting->label);
			remove_meeting( graph, i);
                  }
            }
            else i++;
    }

    
  
    return graph;
}


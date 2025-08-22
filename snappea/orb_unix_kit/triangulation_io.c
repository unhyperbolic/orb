#include <stdio.h>
#include <stdlib.h>

#include "triangulation_io.h"

#include "positioned_tet.h"

#include "kernel_prototypes.h"

#include "tables.h"

#define NL(f)   (f==0) ? 'u' : ((f==1) ? 'v' : ((f==2) ? 'w' : 'x'))

Boolean	contains_flat_tetrahedra( Triangulation *manifold );

char *
triangulation_to_string(
    Triangulation * manifold,
    Boolean ae,
    Boolean ex,
    Boolean curves)
{
    char * buffer = my_malloc(10000000);

    char * p = buffer;
    
	int		index;
	Tetrahedron	*tet;
	EdgeClass	*edge;
	PositionedTet	ptet0,
			ptet;

	for(	tet = manifold->tet_list_begin.next, index = 1;
		tet!=&manifold->tet_list_end;
		tet = tet->next, index++ )
		tet->index = index;

	if (ex)
	{
		if (manifold->solution_type[complete] == geometric_solution )
		{
		    if (contains_flat_tetrahedra(manifold)==TRUE)
		    {
			p += sprintf(p, "SolutionType partially_flat_solution\n");
		    }
		    else
		    {
			p += sprintf(p, "SolutionType geometric_solution\n");
		    }
		}

		if (manifold->solution_type[complete] == nongeometric_solution )
		{
		    p += sprintf(p, "SolutionType nongeometric_solution\n");
		}

		if (manifold->solution_type[complete] == not_attempted )
		    p += sprintf(p, "SolutionType not_attempted\n");

		if (manifold->solution_type[complete] == other_solution )
		    p += sprintf(p, "SolutionType other_solution\n");

		if (manifold->solution_type[complete] == step_failed )
		    p += sprintf(p, "SolutionType step_failed\n");

		if (manifold->solution_type[complete] == no_solution )
		    p += sprintf(p, "SolutionType no_solution\n");

		if (manifold->solution_type[complete] == invalid_solution )
		    p += sprintf(p, "SolutionType invalid_solution\n");

		if (manifold->solution_type[complete] == degenerate_solution )
		    p += sprintf(p, "SolutionType degenerate_solution\n");

		if (manifold->solution_type[complete] == flat_solution )
		    p += sprintf(p, "SolutionType flat_solution\n");

		p += sprintf(p, "vertices_known\n\n");
	}

	for (	edge = manifold->edge_list_begin.next, index = 1;
		edge!=&manifold->edge_list_end;		
		edge = edge->next, index++)
	{
	    p += sprintf(p, "%3d %2d", index, edge->singular_index + 1);

	    p += sprintf(p, " %04.3f", edge->singular_order );

	    set_left_edge(edge,&ptet0);
	    ptet = ptet0;
	    
	    if (ae)
	    {
			double err =  (edge->singular_order==0)
				? 0
				: TWO_PI / edge->singular_order;

			do{
				err -= ptet.tet->dihedral_angle[ultimate]
						[edge_between_faces[ptet.near_face][ptet.left_face]];

				veer_left(&ptet);
			}while (!same_positioned_tet(&ptet, &ptet0));

			p += sprintf(p, " %21.16f", err );
		}

		if (ex)
		{
			if (ptet.tet->cusp[remaining_face[ptet.left_face][ptet.near_face]]->index > -1 )
				p += sprintf(p, " %2d", ptet.tet->cusp[remaining_face[ptet.left_face][ptet.near_face]]->index + 1 );
			else	p += sprintf(p, " %2d", ptet.tet->cusp[remaining_face[ptet.left_face][ptet.near_face]]->index );

			if (ptet.tet->cusp[remaining_face[ptet.near_face][ptet.left_face]]->index > -1 )
				p += sprintf(p, " %2d", ptet.tet->cusp[remaining_face[ptet.near_face][ptet.left_face]]->index + 1 );
			else    p += sprintf(p, " %2d", ptet.tet->cusp[remaining_face[ptet.near_face][ptet.left_face]]->index );
		}

		do{
			char c = NL(ptet.left_face);
			char d = NL(ptet.near_face);

			p += sprintf(p, " %2d%c%c",ptet.tet->index,c,d);
	
			veer_left(&ptet);

		}while (!same_positioned_tet(&ptet, &ptet0));

		p += sprintf(p, "\n");
	}

	if (ex && manifold->solution_type[complete] != not_attempted )
	{
		p += sprintf(p, "\n");

		for (   edge = manifold->edge_list_begin.next, index = 1;
			edge!=&manifold->edge_list_end;
			edge = edge->next, index++)
		{
		    p += sprintf(p, "%3d", index);

			set_left_edge(edge,&ptet0);
			ptet = ptet0;

			p += sprintf(p, " %21.16f", edge->inner_product[ultimate] );

			int top = remaining_face[ptet.left_face][ptet.near_face];
                        p += sprintf(p, " %21.16f", ptet.tet->cusp[top]->inner_product[ultimate] );

                        int bottom = remaining_face[ptet.near_face][ptet.left_face];
                        p += sprintf(p, " %21.16f", ptet.tet->cusp[bottom]->inner_product[ultimate] );

			do
			{
				p += sprintf(p, " %21.16f", ptet.tet->dihedral_angle[ultimate]
						[edge_between_faces[ptet.near_face][ptet.left_face]]);
				veer_left(&ptet);
			}while (!same_positioned_tet(&ptet, &ptet0));

			p += sprintf(p, "\n");
		}
	}

	if (ex && curves )	
	{
                p += sprintf(p, "\n");

                for (   edge = manifold->edge_list_begin.next, index = 1;
                        edge!=&manifold->edge_list_end;
                        edge = edge->next, index++)
                {
		    p += sprintf(p, "%3d", index);

                        set_left_edge(edge,&ptet0);
                        ptet = ptet0;

                        do
                        {
				int top = remaining_face[ptet.left_face][ptet.near_face];
				int bottom = remaining_face[ptet.near_face][ptet.left_face];

                                p += sprintf(p, "   %2d %2d %2d %2d %2d %2d %2d %2d",
					ptet.tet->curve[0][0][top][bottom],
					ptet.tet->curve[0][0][bottom][top],
					ptet.tet->curve[0][1][top][bottom],
					ptet.tet->curve[0][1][bottom][top],
					ptet.tet->curve[1][0][top][bottom],
					ptet.tet->curve[1][0][bottom][top],
					ptet.tet->curve[1][1][top][bottom],
					ptet.tet->curve[1][1][bottom][top] );
                                veer_left(&ptet);
                        }while (!same_positioned_tet(&ptet, &ptet0));

                        p += sprintf(p, "\n");
                }
	}

	return buffer;
}

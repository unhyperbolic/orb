#include "diagram_canvas.h"

#include <qpoint.h>
#include <complex>
#include <cstdio>

EndData::EndData( Edge *e, EndType t )
{
   edge = e;
   type = t;
   angle = 0; 
   singular = FALSE;
}

Vertex::Vertex( QPoint point )
{
    position = point;
    link_id = -1;
}

Vertex::~Vertex()
{
    for(int i=0; i< (int) incidentEndData.size(); i++)
                incidentEndData[i]->~EndData();
    incidentEndData.clear();
}

Edge::Edge( Vertex *vertex1, Vertex *vertex2 )
{
    vertex[begin_] = vertex1;
    vertex[end_] = vertex2;

    cuff_id = -1;
    thickness = THIN;
    selected = FALSE;

    EndData *ed1 = new EndData( this, begin_ );
    EndData *ed2 = new EndData( this, end_ );

    vertex1->incidentEndData.insert( vertex1->incidentEndData.end(), ed1 );

    vertex2->incidentEndData.insert( vertex2->incidentEndData.end(), ed2 );

}

Edge::~Edge()
{
	underpasses.clear();
}

void Edge::compute_equation_coefficients()
{
    a = vertex[end_]->position.y() - vertex[begin_]->position.y();
    b = vertex[begin_]->position.x() - vertex[end_]->position.x();
    c = -a*vertex[begin_]->position.x() -  b*vertex[begin_]->position.y();
}


double Edge::distance_from_edge( QPoint point )
{
    compute_equation_coefficients();

    double x0 = (double) point.x();
    double y0 = (double) point.y();

    if ( a == 0 && b == 0 )
    {
        double u = (double) vertex[begin_]->position.x();
        double v = (double) vertex[begin_]->position.y();

        return sqrt( (x0-u)*(x0-u) + (y0-v)*(y0-v) );
    }
    else
        return abs( a*x0 + b*y0 + c ) / sqrt( double (a*a + b*b) );
}


double Edge::projection_to_edge( QPoint point )
{
    compute_equation_coefficients();

    if ( a == 0 && b == 0 ) return 0;

    double x0 = (double) point.x();
    double y0 = (double) point.y();

    double x = ( b*b*x0 - a*b*y0 - a*c ) / ( a*a + b*b );
    double y = ( -a*b*x0 + a*a*y0 - b*c ) / ( a*a + b*b );

    if ( abs((double) (vertex[begin_]->position.x() - vertex[end_]->position.x()) ) >= 5 )
        return ( x - vertex[begin_]->position.x() ) / ( vertex[end_]->position.x() - vertex[begin_]->position.x() );
    else
        return ( y - vertex[begin_]->position.y() ) / ( vertex[end_]->position.y() - vertex[begin_]->position.y() );
}




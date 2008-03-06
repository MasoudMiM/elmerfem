#include <iostream>
#include <math.h>
#include "meshutils.h"
using namespace std;

Meshutils::Meshutils()
{
}


Meshutils::~Meshutils()
{
}


// Bounding box...
//-----------------------------------------------------------------------------
double* Meshutils::boundingBox(mesh_t *mesh)
{
  double xmin = +9e9;
  double xmax = -9e9;

  double ymin = +9e9;
  double ymax = -9e9;

  double zmin = +9e9;
  double zmax = -9e9;

  for(int i=0; i < mesh->nodes; i++) {
    node_t *node = &mesh->node[i];
    
    if(node->x[0] > xmax) 
      xmax = node->x[0];
    
    if(node->x[0] < xmin) 
      xmin = node->x[0];
    
    if(node->x[1] > ymax) 
      ymax = node->x[1];

    if(node->x[1] < ymin) 
      ymin = node->x[1];

    if(node->x[2] > zmax) 
      zmax = node->x[2];

    if(node->x[2] < zmin) 
      zmin = node->x[2];
  }
  
  double xmid = (xmin + xmax)/2.0;
  double ymid = (ymin + ymax)/2.0;
  double zmid = (zmin + zmax)/2.0;

  double xlen = (xmax - xmin)/2.0;
  double ylen = (ymax - ymin)/2.0;
  double zlen = (zmax - zmin)/2.0;

  double s = xlen;

  if(ylen > s)
    s = ylen;
  
  if(zlen > s)
    s = zlen;
  
  s *= 1.1;
  
  double *result = new double[10];

  result[0] = xmin;
  result[1] = xmax;
  result[2] = ymin;
  result[3] = ymax;
  result[4] = zmin;
  result[5] = zmax;

  result[6] = xmid;
  result[7] = ymid;
  result[8] = zmid;

  result[9] = s;

  return result;
}

// Delete mesh...
//-----------------------------------------------------------------------------
void Meshutils::clearMesh(mesh_t *mesh)
{
  if(mesh != (mesh_t*)NULL) {
    if(mesh->element != (element_t*)NULL) 
      delete [] mesh->element;
    
    if(mesh->boundaryelement != (boundaryelement_t*)NULL) 
      delete [] mesh->boundaryelement;
    
    if(mesh->edge != (edge_t*)NULL) 
      delete [] mesh->edge;
    
    if(mesh->node != (node_t*)NULL)
      delete [] mesh->node;
    
    delete [] mesh;
  }
}


// Find parents for boundary elements...
//----------------------------------------------------------------------------
void Meshutils::findBoundaryElementParents(mesh_t *mesh)
{
#define UNKNOWN -1

#define RESETENTRY0             \
  h->node[0] = UNKNOWN;		\
  h->node[1] = UNKNOWN;  	\
  h->element[0] = UNKNOWN;	\
  h->element[1] = UNKNOWN;	\
  h->next = NULL;

  class hashEntry {
  public:
    int node[2];
    int element[2];
    hashEntry *next;
  };

  int keys = mesh->nodes;
  
  hashEntry *hash = new hashEntry[keys];

  bool found;
  hashEntry *h;

  for(int i=0; i<keys; i++) {
    h = &hash[i];
    RESETENTRY0;
  }

  static int facemap[][3] = {{0,1,2}, {0,1,3}, {0,2,3}, {1,2,3}};
  
  for(int i=0; i < mesh->elements; i++) {
    element_t *e = &mesh->element[i];

    for(int f=0; f<4; f++) {
      int n0 = e->node[facemap[f][0]];
      int n1 = e->node[facemap[f][1]];
      int n2 = e->node[facemap[f][2]];

      if(n2 < n1) {
	int tmp = n2;
	n2 = n1;
	n1 = tmp;
      }
      
      if(n2 < n0) {
	int tmp = n2;
	n2 = n0;
	n0 = tmp;
      }
      
      if(n1 < n0) {
	int tmp = n1;
	n1 = n0;
	n0 = tmp;
      }
      
      h = &hash[n0];
      found = false;
      while(h->next) {                                       
	if((h->node[0] == n1) && (h->node[1] == n2)) {
	  found = true;
	  break;
	}
	h = h->next;
      }                                                      
      
      if(!found) {
	h->node[0] = n1;
	h->node[1] = n2;
	h->element[0] = i;
	h->next = new hashEntry;
	h = h->next;
	RESETENTRY0;
      } else {
	h->element[1] = i;
      }      
    }
  }

  // count faces:
  int faces = 0;
  for(int i=0; i<keys; i++) {
    h = &hash[i];
    while(h = h->next) 
      faces++;
  }
  
  cout << "Found total of " << faces << " faces" << endl;

  // Finally find parents:
  for(int i=0; i < mesh->boundaryelements; i++) {
    boundaryelement_t *be = &mesh->boundaryelement[i];
    
    int n0 = be->node[0];
    int n1 = be->node[1];
    int n2 = be->node[2];
    
    if(n2 < n1) {
      int tmp = n2;
      n2 = n1;
      n1 = tmp;
    }
    
    if(n2 < n0) {
      int tmp = n2;
      n2 = n0;
      n0 = tmp;
    }
    
    if(n1 < n0) {
      int tmp = n1;
      n1 = n0;
      n0 = tmp;
    }
    
    h = &hash[n0];
    while(h->next) {
      if((h->node[0] == n1) && (h->node[1] == n2)) {
	be->element[0] = h->element[0];
	be->element[1] = h->element[1];
      }
      h = h->next;
    }
  }

  delete [] hash;
}


// Find edges for boundary elements...
//-----------------------------------------------------------------------------
void Meshutils::findBoundaryElementEdges(mesh_t *mesh)
{
#define UNKNOWN -1

#define RESETENTRY              \
    h->node = UNKNOWN;          \
    h->boundaryelements = 0;    \
    h->boundaryelement = NULL;  \
    h->next = NULL;

  int keys = mesh->nodes;

  class hashEntry {
  public:
    int node;
    int boundaryelements;
    int *boundaryelement;
    hashEntry *next;
  };
  
  hashEntry *hash = new hashEntry[keys];

  bool found;
  hashEntry *h;

  for(int i=0; i<keys; i++) {
    h = &hash[i];
    RESETENTRY;
  }

  static int edgemap[][2] = {{0,1}, {1,2}, {2,0}};
  
  for(int i=0; i < mesh->boundaryelements; i++) {
    boundaryelement_t *be = &mesh->boundaryelement[i];
    
    // loop over edges
    for(int e=0; e<3; e++) {

      int n0 = be->node[edgemap[e][0]];
      int n1 = be->node[edgemap[e][1]];

      int m = (n0<n1) ? n0 : n1;
      int n = (n0<n1) ? n1 : n0;

      h = &hash[m];
      found = false;
      while(h->next) {                                       
	if(h->node == n) {
	  found = true;
	  break;
	}
	h = h->next;
      }                                                      
      
      if(!found) {
	h->node = n;
	h->boundaryelements = 1;
	h->boundaryelement = new int[1];
	h->boundaryelement[0] = i;
	h->next = new hashEntry;
	h = h->next;
	RESETENTRY;
      } else {
	int *tmp = new int[h->boundaryelements];
	for(int j=0; j<h->boundaryelements; j++)
	  tmp[j] = h->boundaryelement[j];
	delete [] h->boundaryelement;
	h->boundaryelement = new int[h->boundaryelements+1];
	for(int j=0; j<h->boundaryelements; j++)
	  h->boundaryelement[j] = tmp[j];
	h->boundaryelement[h->boundaryelements++] = i;
	delete [] tmp;
      }
    }
  }

  // count edges:
  int edges = 0;
  for(int i=0; i<keys; i++) {
    h = &hash[i];
    while(h = h->next) 
      edges++;
  }

  cout << "Found " << edges << " edges on boundary" << endl;

  mesh->edges = edges;
  // delete [] mesh->edge;
  mesh->edge = new edge_t[edges];

  // Create edges:
  edges = 0;
  for(int i=0; i<keys; i++) {
    h = &hash[i];
    while(h->next) {
      edge_t *e = &mesh->edge[edges++];
      
      e->code = 202;

      e->nodes = 2;
      e->node = new int[2];

      e->node[0] = i;
      e->node[1] = h->node;

      e->boundaryelements = h->boundaryelements;
      e->boundaryelement = new int[e->boundaryelements];

      for(int j=0; j < e->boundaryelements; j++) {
	e->boundaryelement[j] = h->boundaryelement[j];
      }

      e->index = UNKNOWN;
      h = h->next;
    }
  }

  delete [] hash;

  // Inverse map
  for(int i=0; i < mesh->edges; i++) {
    edge_t *e = &mesh->edge[i];

    for(int j=0; j < e->boundaryelements; j++) {
      int k = e->boundaryelement[j];
      boundaryelement_t *be = &mesh->boundaryelement[k];
      
      for(int r=0; r < be->edges; r++) {
	if(be->edge[r] < 0) {
	  be->edge[r] = i;
	  break;
	}
      }
    }
  }  

#if 0
  cout << "*********************" << endl;
  for(int i=0; i<mesh->edges; i++)
    cout << "Edge " << i << " nodes " << mesh->edge[i].node[0] << " "<< mesh->edge[i].node[0] << endl;

  for(int i=0; i<mesh->boundaryelements; i++)
    cout << "Boundaryelement " << i << " nodes " 
	 << mesh->boundaryelement[i].node[0] << " " 
	 << mesh->boundaryelement[i].node[1] << " "
	 << mesh->boundaryelement[i].node[2] << " "
	 << " Edges " 
	 << mesh->boundaryelement[i].edge[0] << " " 
	 << mesh->boundaryelement[i].edge[1] << " "
	 << mesh->boundaryelement[i].edge[2] << " "
	 << " Parents " 
	 << mesh->boundaryelement[i].element[0] << " " 
	 << mesh->boundaryelement[i].element[1] << " "
	 << endl;

  cout.flush();
#endif

}


// Find sharp edges for boundary elements...
//-----------------------------------------------------------------------------
void Meshutils::findSharpEdges(mesh_t *mesh, double limit)
{
#define PI 3.14159
#define UNKNOWN -1
#define SHARP 0
  
  cout << "Limit: " << limit << " degrees" << endl;
  cout.flush();

  double angle;
  int count = 0;
  
  for(int i=0; i<mesh->edges; i++) {
    edge_t *edge = &mesh->edge[i];

    edge->index = UNKNOWN;

    if( edge->boundaryelements == 2 ) {
      int b0 = edge->boundaryelement[0];
      int b1 = edge->boundaryelement[1];    
      double *n0 = mesh->boundaryelement[b0].normal;
      double *n1 = mesh->boundaryelement[b1].normal;
      double cosofangle = n0[0]*n1[0] + n0[1]*n1[1] + n0[2]*n1[2];
      angle = acos(cosofangle) / PI * 180.0;
    } else {
      angle = 180.0;
    }    
    
    if(sqrt(angle*angle) > limit) {
      edge->index = SHARP;
      count++;
    }
  }

  cout << "Found " << count << " sharp edges" << endl;
}


// Divide boundary by sharp edges...
//-----------------------------------------------------------------------------
int Meshutils::divideBoundaryBySharpEdges(mesh_t *mesh)
{
#define UNKNOWN -1
#define SHARP 0

  class Bc {
  public:
    void propagateIndex(mesh_t* mesh, int index, int i) {
      boundaryelement_t *boundaryelement = &mesh->boundaryelement[i];

      // index is ok
      if(boundaryelement->index != UNKNOWN)
	return;

      // set index
      boundaryelement->index = index;

      // propagate index
      for(int j=0; j<3; j++) {
	int k = boundaryelement->edge[j];
	edge_t *edge = &mesh->edge[k];

	// skip sharp edges
	if(edge->index != SHARP) {
	  for(int m=0; m < edge->boundaryelements; m++) {
	    int n = edge->boundaryelement[m];
	    propagateIndex(mesh, index, n);
	  }
	}
      }
    }
  };
  
  Bc *bc = new Bc;
  
  // reset bc-indices:
  for(int i=0; i < mesh->boundaryelements; i++)
    mesh->boundaryelement[i].index = UNKNOWN;

  // recursively determine boundary parts:
  int index = 0;
  for(int i=0; i < mesh->boundaryelements; i++) {
    boundaryelement_t *boundaryelement = &mesh->boundaryelement[i];
    if(boundaryelement->index == UNKNOWN) {
      index++;
      bc->propagateIndex(mesh, index, i);
    }
  }

  cout << "Divided boudary into " << index << " parts" << endl;

  delete bc;

  return index;
}

// Divide boundary by sharp edges...
//-----------------------------------------------------------------------------
void Meshutils::findBoundaryElementNormals(mesh_t *mesh)
{
  static double a[3], b[3], c[3];
  double center_boundaryelement[3], center_element[3], center_difference[3];
  Helpers *helpers = new Helpers;
  int u, v, w, q, e0, e1, i0, i1, bigger;

  for(int i=0; i < mesh->boundaryelements; i++) {
    boundaryelement_t *boundaryelement = &mesh->boundaryelement[i];
    
    u = boundaryelement->node[0];
    v = boundaryelement->node[1];
    w = boundaryelement->node[2];

    // Calculate normal (modulo sign):
    a[0] = mesh->node[v].x[0] - mesh->node[u].x[0];
    a[1] = mesh->node[v].x[1] - mesh->node[u].x[1];
    a[2] = mesh->node[v].x[2] - mesh->node[u].x[2];
    
    b[0] = mesh->node[w].x[0] - mesh->node[u].x[0];
    b[1] = mesh->node[w].x[1] - mesh->node[u].x[1];
    b[2] = mesh->node[w].x[2] - mesh->node[u].x[2];
    
    helpers->crossProduct(a,b,c);
    helpers->normalize(c);
    
    boundaryelement->normal[0] = c[0];
    boundaryelement->normal[1] = c[1];
    boundaryelement->normal[2] = c[2];
    
    // Determine sign:
    //----------------
    // a) which parent element has bigger index?
    e0 = boundaryelement->element[0];
    e1 = boundaryelement->element[1];
    
    if( (e0<0) && (e1<0) ) {
      // both parents unknown
      bigger = -1;
    } else if(e1<0) {
      // e0 known, e1 unknown
      bigger = e0;
    } else {
      // both parents known
      bigger = e0;
      i0 = mesh->element[e0].index;
      i1 = mesh->element[e1].index;
      if(i1 > i0)
	bigger = e1;
    }
    
    // b) normal should points from bigger to smaller parent index:
    if(bigger > -1) {
      center_boundaryelement[0] = mesh->node[u].x[0] 
	                        + mesh->node[v].x[0]
                                + mesh->node[w].x[0];
      center_boundaryelement[1] = mesh->node[u].x[1]
                                + mesh->node[v].x[1]
                                + mesh->node[w].x[1];
      center_boundaryelement[2] = mesh->node[u].x[2]
                                + mesh->node[v].x[2]
                                + mesh->node[w].x[2];
      
      center_boundaryelement[0] /= 3.0;
      center_boundaryelement[1] /= 3.0;
      center_boundaryelement[2] /= 3.0;
      
      element_t *e = &mesh->element[bigger];
      
      u = e->node[0];
      v = e->node[1];
      w = e->node[2];
      q = e->node[3];
      
      center_element[0] = mesh->node[u].x[0]
                        + mesh->node[v].x[0]
                        + mesh->node[w].x[0]
                        + mesh->node[q].x[0];
      center_element[1] = mesh->node[u].x[1]
                        + mesh->node[v].x[1]
                        + mesh->node[w].x[1]
                        + mesh->node[q].x[1];
      center_element[2] = mesh->node[u].x[2]
                        + mesh->node[v].x[2]
                        + mesh->node[w].x[2]
                        + mesh->node[q].x[2];
      
      center_element[0] /= 4.0;
      center_element[1] /= 4.0;
      center_element[2] /= 4.0;
      
      center_difference[0] = center_element[0] - center_boundaryelement[0];
      center_difference[1] = center_element[1] - center_boundaryelement[1];
      center_difference[2] = center_element[2] - center_boundaryelement[2];
      
      // dot product must be negative
      double dp = center_difference[0]*c[0]
                + center_difference[1]*c[1] 
                + center_difference[2]*c[2];
      
      if(dp > 0.0) {
	boundaryelement->normal[0] = -c[0];
	boundaryelement->normal[1] = -c[1];
	boundaryelement->normal[2] = -c[2];

      }
    }
  }

  delete helpers;
}

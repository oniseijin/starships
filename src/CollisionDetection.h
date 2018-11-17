//
//  CollisionDetection.h
//
//  Created by Ryan Mills on 2014-04-24.
//
//

#ifndef __CollisionDetection__
#define __CollisionDetection__

#include <iostream>
#include <cmath>





/*
 POINT/RECT COLLISION DETECTION
 Jeff Thompson
 Fall 2011
 
 www.jeffreythompson.org
 */


/*
 POINT/RECT COLLISION FUNCTION
 Takes 6 arguments:
 + x,y position of point 1 - in this case "you"
 + x,y position of point 2 - in this case the static rectangle
 + width and height of rectangle
 */
bool pointRect(int px, int py, int rx, int ry, int rw, int rh) {
    
    // test for collision
    if (px >= rx-rw/2 && px <= rx+rw/2 && py >= ry-rh/2 && py <= ry+rh/2) {
        return true;    // if a hit, return true
    }
    else {            // if not, return false
        return false;
    }
}

/*
 BALL/BALL COLLISION FUNCTION
 Jeff Thompson // v0.9 // November 2011 // www.jeffreythompson.org
 
 Takes 6 arguments:
 + x,y position of the first ball - in this case "you"
 + diameter of first ball - elliptical collision is VERY difficult
 + x,y position of the second ball
 + diameter of second ball
 
 */
bool ballBall(int x1, int y1, int d1, int x2, int y2, int d2) {
    
    // find distance between the two objects
    float xDist = x1-x2;                                   // distance horiz
    float yDist = y1-y2;                                   // distance vert
    float distance = sqrt((xDist*xDist) + (yDist*yDist));  // diagonal distance
    
    // test for collision
    if (d1/2 + d2/2 > distance) {
        return true;    // if a hit, return true
    }
    else {            // if not, return false
        return false;
    }
}

/*
 BALL/LINE COLLISION FUNCTION
 Jeff Thompson // v0.9 // November 2011 // www.jeffreythompson.org
 
 Based on the example by Philip Nicoletti
 http://www.codeguru.com/forum/showthread.php?threadid=194400
 
 Takes 7 arguments:
 + x,y position of the point
 + diameter of ball (assumes a circle - ellipse collision is REALLY hard)
 + start x,y and end x,y of the line
 
 Note: all values must be floats, otherwise rounding from ints will cause
 errors on one side of the line
 */

bool ballLine(float bx, float by, int d, float lx1, float ly1, float lx2, float ly2) {
    
    // first get the length of the line using the Pythagorean theorem
    float distX = lx1-lx2;
    float distY = ly1-ly2;
    float lineLength = sqrtf((distX*distX) + (distY*distY));
    
    // then solve for r
    float r = (((bx-lx1)*(lx2-lx1))+((by-ly1)*(ly2-ly1)))/pow(lineLength, 2);
    
    // get x,y points of the closest point
    float closestX = lx1 + r*(lx2-lx1);
    float closestY = ly1 + r*(ly2-ly1);
    
    // to get the length of the line, use the Pythagorean theorem again
    float distToPointX = closestX - bx;
    float distToPointY = closestY - by;
    float distToPoint = sqrtf(pow(distToPointX, 2) + pow(distToPointY, 2));
    
    // for explanation purposes, draw a line to the ball from the closest point
    //strokeWeight(1);
    //stroke(255,0,0);
    //line(closestX, closestY, bx, by);
    //strokeWeight(3);
    
    // if that distance is less than the radius of the ball: collision
    if (distToPoint <= d/2) {
        // hit
        return true;
    }
    else {
        return false;
    }
}

/*
 LINE/LINE COLLISION FUNCTION
 Jeff Thompson // v0.9 // November 2011 // www.jeffreythompson.org
 
 Based on the tutorial by Paul Bourke (thanks!):
 http://paulbourke.net/geometry/lineline2d
 ... and Ibackstrom (thanks!)
 http://community.topcoder.com/tc?module=Static&d1=tutorials&d2=geometry2
 
 Takes 8 arguments:
 + x,y positions of start and end of one line (in this case, the moving one)
 + x,y positions of start and end of the other line (in this case, the fixed one)
 
 Note: all values must be floats, otherwise rounding from ints will cause
 errors on one side of the line
 */

bool lineLine(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    
    // find uA and uB
    float uA = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
    float uB = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
    
    // note: if the below equations is true, the lines are parallel
    // ... this is the denominator of the above equations
    // (y4-y3)*(x2-x1) - (x4-x3)*(y2-y1)
    
    if (uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1) {
        
        // find intersection point, if desired
        //float intersectionX = x1 + (uA * (x2-x1));
        //float intersectionY = y1 + (uA * (y2-y1));
        // comment out draw
        //noStroke();
        //fill(0);
        //ellipse(intersectionX, intersectionY, 10,10);
        
        
        return true;
    }
    else {
        return false;
    }
}


/*
 LINE/PLANE COLLISION - 3d
 Jeff Thompson
 
 Line to plane collision in 3d.  Use mouse to rotate view, arrow
 keys to move the line and watch the results!
 
 Based on examples from:
 http://gmc.yoyogames.com/index.php?showtopic=501626
 
 www.jeffreythompson.org
 */



// three points on the plane, followed by ends of the line (x,y,z)
// pass back the intersection
float* intersect(float intersection[], float ax, float ay, float az, float bx, float by, float bz, float cx, float cy, float cz, float px, float py, float pz, float qx, float qy, float qz) {
    
    //float intersection[] = new float;{0,0,0};
    
    float sx;
    float sy;
    float sz;
    // vector from A to C
    float ex = cx-ax;
    float ey = cy-ay;
    float ez = cz-az;
    
    // vector from A to B
    float fx = bx-ax;
    float fy = by-ay;
    float fz = bz-az;
    
    // cross product of E and F
    float nx = fy*ez-fz*ey;
    float ny = fz*ex-fx*ez;
    float nz = fx*ey-fy*ex;
    
    // normalize
    float m = sqrtf(nx*nx+ny*ny+nz*nz);
    nx /= m;
    ny /= m;
    nz /= m;
    
    //get ray direction vector
    float dx = qx-px;
    float dy = qy-py;
    float dz = qz-pz;
    
    m = sqrtf(dx*dx+dy*dy+dz*dz);
    dx /= m;
    dy /= m;
    dz /= m;
    
    // dot product of ray and normal
    // this tells us if the ray is pointing towards the plane
    m = dx*nx + dy*ny + dz*nz;
    
    // less than 0 means the vectors are opposed
    // more than 0 means the vectors point the same way
    // equal to 0 means they are perpendicular
    if (m < 0) {
        
        // get the vector from the ray to point A on the plane
        float gx = ax-px;
        float gy = ay-py;
        float gz = az-pz;
        
        // dot product of G and plane normal
        float t = gx*nx+gy*ny+gz*nz;
        
        // if we are on non-culled side of the plane
        if (t < 0) {
            // get distance to plane by dividing the two dot products
            float k = t/m;
            
            // find the point of intersection on the plane, return result
            sx = px+dx*k;
            sy = py+dy*k;
            sz = pz+dz*k;
            intersection[0] = sx;
            intersection[1] = sy;
            intersection[2] = sz;
        }
    }
    
    return intersection;
}

/*
 POINT/BALL COLLISION FUNCTION
 Jeff Thompson // v0.9 // November 2011 // www.jeffreythompson.org
 
 Takes 5 arguments:
 + x,y position of the point - in this case "you"
 + x,y position of the ball
 + diameter of ball - elliptical collision is VERY difficult
 */
bool pointBall(float px, float py, float bx, float by, int bSize) {
    
    // find distance between the two objects
    float xDist = px-bx;                                   // distance horiz
    float yDist = py-by;                                   // distance vert
    float distance = sqrtf((xDist*xDist) + (yDist*yDist));  // diagonal distance
    
    // test for collision
    if (bSize/2 > distance) {
        return true;    // if a hit, return true
    }
    else {            // if not, return false
        return false;
    }
}

/*
 POINT/LINE COLLISION FUNCTION
 Jeff Thompson // v0.9 // November 2011 // www.jeffreythompson.org
 
 Takes 6 arguments:
 + x,y position of the point
 + start x,y and end x,y of the line
 
 Note: all values must be floats, otherwise rounding from ints will cause
 errors on one side of the line
 */

bool pointLine(float px, float py, float lx1, float ly1, float lx2, float ly2) {
    
    // get the slope of the entire line
    float lineSlope = (ly2-ly1)/(lx2-lx1);
    
    // get slope from one end of the line to the point
    float pointSlope = (ly2-py)/(lx2-px);
    
    // if the slopes are the same, then the point is on the line!
    if (lineSlope == pointSlope) {
        return true;
    }
    else {
        return false;
    }
    
}

/*
 POINT/POINT COLLISION FUNCTION
 Jeff Thompson // v0.9 // November 2011 // www.jeffreythompson.org
 
 Takes 4 arguments:
 + x,y position of point 1 - in this case "you"
 + x,y position of point 2 - in this case the static point
 */
bool pointPoint(int x1, int y1, int x2, int y2) {
    
    // test for collision
    if (x1 == x2 && y1 == y2) {
        return true;    // if a hit, return true
    }
    else {            // if not, return false
        return false;
    }
}

/*
 POINT/POLYGON COLLISION DETECTION
 Jeff Thompson | 2013 | www.jeffreythompson.org
 
 Finds collisions betwen a point and a N-sided polygon - very
 flexible!
 
 Via: http://stackoverflow.com/a/2922778/1167783
 
 Takes 5 arguments:
 + # of vertices
 + float array x and y coordinates for vertices
 + x/y coordinates for point
 */
bool pointPolygon (int numVertices, float vertX[], float vertY[], float px, float py) {
    bool collision = false;
    for (int i=0, j=numVertices-1; i < numVertices; j = i++) {
        if ( ((vertY[i]>py) != (vertY[j]>py)) && (px < (vertX[j]-vertX[i]) * (py-vertY[i]) / (vertY[j]-vertY[i]) + vertX[i]) ) {
            collision = !collision;
        }
    }
    return collision;
}

/*
 POINT/TRIANGLE COLLISION DETECTION
 Jeff Thompson | 2013 | www.jeffreythompson.org
 
 Takes 2 sets of arguments:
 + x/y coordinates for triangle
 + x/y coordinates for the point
 
 Built using a modified version of this post:
 http://gmc.yoyogames.com/index.php?showtopic=106307
 */

bool pointTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int px, int py) {
    int a0 = abs((x2-x1)*(y3-y1)-(x3-x1)*(y2-y1));
    int a1 = abs((x1-px)*(y2-py)-(x2-px)*(y1-py));
    int a2 = abs((x2-px)*(y3-py)-(x3-px)*(y2-py));
    int a3 = abs((x3-px)*(y1-py)-(x1-px)*(y3-py));
    
    return (abs(a1+a2+a3 - a0) <= 1/256);
}
/*
 RECT/BALL COLLISION FUNCTION
 Jeff Thompson // v0.9 // November 2011 // www.jeffreythompson.org
 
 Actually quite a bit harder than it looks!
 Built from an example by Matt Worden (http://vband3d.tripod.com/visualbasic/tut_mixedcollisions.htm)
 
 Takes 7 arguments:
 + x,y position of the first ball - in this case "you"
 + width and height of rect
 + x,y position of the second ball
 + diameter of second ball
 
 */

bool rectBall(int rx, int ry, int rw, int rh, int bx, int by, int d) {
    
    // first test the edges (this is necessary if the rectangle is larger
    // than the ball) - do this with the Pythagorean theorem
    
    // if ball entire width position is between rect L/R sides
    if (bx+d/2 >= rx-rw/2 && bx-d/2 <= rx+rw/2 && abs(ry-by) <= d/2) {
        return true;
    }
    // if not, check if ball's entire height is between top/bottom of the rect
    else if (by+d/2 >= ry-rh/2 && by-d/2 <= ry+rh/2 && abs(rx-bx) <= d/2) {
        return true;
    }
    
    // if that doesn't return a hit, find closest corner
    // this is really just a point, so we can test if we've hit it
    // upper-left
    float xDist = (rx-rw/2) - bx;  // same as ball/ball, but first value defines point, not center
    float yDist = (ry-rh/2) - by;
    float shortestDist = sqrtf((xDist*xDist) + (yDist * yDist));
    
    // upper-right
    xDist = (rx+rw/2) - bx;
    yDist = (ry-rh/2) - by;
    float distanceUR = sqrtf((xDist*xDist) + (yDist * yDist));
    if (distanceUR < shortestDist) {  // if this new distance is shorter...
        shortestDist = distanceUR;      // ... update
    }
    
    // lower-right
    xDist = (rx+rw/2) - bx;
    yDist = (ry+rh/2) - by;
    float distanceLR = sqrtf((xDist*xDist) + (yDist * yDist));
    if (distanceLR < shortestDist) {
        shortestDist = distanceLR;
    }
    
    // lower-left
    xDist = (rx-rw/2) - bx;
    yDist = (ry+rh/2) - by;
    float distanceLL = sqrtf((xDist*xDist) + (yDist * yDist));
    if (distanceLL < shortestDist) {
        shortestDist = distanceLL;
    }
    
    // test for collision
    if (shortestDist < d/2) {  // if less than radius
        return true;             // return true
    }
    else {                     // otherwise, return false
        return false;
    }
}

/* 
 RECT/RECT COLLISION FUNCTION
 Jeff Thompson // v0.9 // November 2011 // www.jeffreythompson.org
 
 Takes 8 arguments:
 + x,y position of object 1 - in this case "you"
 + width and height of object 1 - also "you"
 + x,y position of object 2 - in this case the static rectangle
 + width and height of object 2
 
 */
bool rectRect(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    
    // test for collision
    if (x1+w1/2 >= x2-w2/2 && x1-w1/2 <= x2+w2/2 && y1+h1/2 >= y2-h2/2 && y1-h1/2 <= y2+h2/2) {
        return true;    // if a hit, return true
    }
    else {            // if not, return false
        return false;
    }
}


#endif /* defined(__Starships__CollisionDetection__) */

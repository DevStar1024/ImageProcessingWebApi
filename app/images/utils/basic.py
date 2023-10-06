import math
import numpy as np

class Point():
    def __init__(self):
        self.x = 0
        self.y = 0
    def __init__(self, x, y):
        self.x = x
        self.y = y
    def setX(self,x):
        self.x = x 
    def setY(self, y):
        self.y = y


class Line():
    def __init__(self):
        self.p1 = Point()
        self.p2 = Point()
    def __init__(self,st:Point,en:Point):
        self.p1 = st
        self.p2 = en
    
    def setStPoint(self,st:Point):
        self.p1 = st
    def setEnPoint(self,en:Point):
        self.p2 = en

    #return the length of the line
    def length(self):
        return calculate_Line_Length(self.p1,self.p2)
    #angle counterclockwise from the horizontal, in degress
    def angle(self):
        x = self.p2.x - self.p1.x
        y = self.p2.y - self.p1.y
        radians = math.atan2(y, x)
        angle = math.degrees(radians)
        if angle < 0:
            angle += 360
        return angle

    
def angle_Degree2Radian(angle):
    return angle * math.pi / 180


def calculate_Line_Length(a:Point, b:Point):
    distance = math.sqrt((a.x-b.x) * (a.x-b.x) + (a.y-b.y) *(a.y-b.y))
    return distance





def angle_Angle_Between_Three_Points(a:Point, b:Point, c:Point):
    v1 = (a.x - b.x, a.y - b.y)
    v2 = (c.x - b.x, c.y - b.y)

    #calculate the dot product between the vectors
    dot_product = v1[0]*v2[0] + v1[1]*v2[1]

    # Calculate the magnitudes of the vectors
    mag_v1 = math.sqrt(v1[0]**2 + v1[1]**2)
    mag_v2 = math.sqrt(v2[0]**2 + v2[1]**2)

    value =  dot_product / (mag_v1 * mag_v2)
    if(math.fabs(value) > 1):
        return 0

    # Calculate the angle between the vectors using the dot product and magnitudes
    angle = math.acos(dot_product / (mag_v1 * mag_v2))

    # Convert the angle from radians to degrees
    angle_degrees = math.degrees(angle)

    return angle_degrees


#X coordinate of arc's center --- angle : center X
def angle_CenterX(a:Point, b:Point, c:Point):
    return b.x

#Y coordinate of arc's center --- angle : center Y
def angle_CenterY(a:Point, b:Point, c:Point) :
    return b.y

#Radius of arc --- angle : radius
def angle_Radius(a:Point, b:Point, c:Point):
    dist1 = calculate_Line_Length(a,b)
    dist2 = calculate_Line_Length(b,c)
    return (dist1 + dist2) / 6.0


#angle clockwise from the horizontal line , in degress --- Line : angle
def line_Angle(line:Line):
    return line.angle()

#X coordinate of the end of the line --- Line : End X
def line_EndX(line:Line):
    return line.p2.x

#Y coordinate of the end of the line --- Line: End Y
def line_EndY(line:Line):
    return line.p2.y

#X coordinate of the start of the line --- Line:Start X
def line_StartX(line:Line):
    return line.p1.x

#Y coordinate of the start of the line --- Line:Start Y
def line_StartY(line:Line):
    return line.p1.y

#Total Length of the line --- Line:Length
def line_Length(line:Line):
    return line.length()

#X coordinate of the mean position of the line --- Line:Position X
def line_PositionX(line:Line):
    return (line.p1.x + line.p2.x) / 2

#Y coordinate of the mean position of the line --- Line:Position Y
def line_PositionY(line:Line):
    return (line.p1.y + line.p2.y) / 2


# X,Y coordinates of the mean position of the line --- Line:Position XY
def line_PositionXY(line:Line):
    return (line_PositionX(line) +  line_PositionY(line)) / 2

#X location of the point
def point_LocationX(p:Point):
    return p.x

#Y location of the point
def point_LocationY(p:Point):
    return p.y

#X,Y location of the point
def point_LocationXY(p:Point):
    return (p.x + p.y) / 2


def line_EndXY(line:Line):
    return (line.p2.x + line.p2.y) / 2

def line_StartXY(line:Line):
    return (line.p1.x + line.p1.y) / 2

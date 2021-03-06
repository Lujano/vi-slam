import csv
import numpy as np
import matplotlib.pyplot as plt
import math


def error(error):
    remap_matrix = np.array([])
    for x in np.arange(error.size):
        error_value = error[x]
        if error[x]>180:
            error_value=error[x]-360
        if error[x]<-180:
            error_value=error[x]+360
        remap_matrix = np.append( remap_matrix, [error_value], 0)
    return remap_matrix 



       
def remap(angle):
    remap_matrix = np.array([])
    for x in np.arange(angle.size):
        angle_value  = angle[x] 

        if angle_value <0:
            angle_value = angle_value+360

        remap_matrix = np.append( remap_matrix, [angle_value], 0)
    return remap_matrix 



def remap3(angle):
    remap_matrix = np.array([])
    for x in np.arange(angle.size):
        angle_value  = angle[x] 

        angle_value = angle_value-360

        remap_matrix = np.append( remap_matrix, [angle_value], 0)
    return remap_matrix 
def remap2(angle):
    remap_matrix = np.array([])
    last_value = angle[0]
    for x in np.arange(angle.size):
        angle_value  = angle[x] 

        if abs(last_value-angle_value)>180:
            angle_value = angle_value+360

        remap_matrix = np.append( remap_matrix, [angle_value], 0)

        last_value = angle_value
    return remap_matrix 
        
def residual3(acc):
    residual_matrix = np.zeros([1, 3])
    rx = 0.0
    ry = 0.0
    rz = 0.0
    for x in np.arange(acc[:, 0].size):
        if  x != 0 :
            rx = acc[x, 0]-acc[x-1, 0]
            ry = acc[x ,1]-acc[x-1, 1]
            rz = acc[x ,2]-acc[x-1, 2]
            residual_matrix = np.append( residual_matrix, [[rx, ry, rz]], 0)
        else:
            residual_matrix = np.append( residual_matrix, [[rx, ry, rz]], 0)


        
    return residual_matrix

def residualAlterado(velocidad, tInicial, dt):
    residualesVelocidad = residual(velocidad)
    residual_matrix = np.array([])
    lastTraslation = tInicial
    for x in np.arange(velocidad.size):
        rx = lastTraslation+residualesVelocidad[x]*dt
        residual_matrix = np.append(residual_matrix, [rx])
        lastTraslation = rx
    return residual_matrix



def residual(acc):
    residual_matrix = np.array([])
    rx = 0.0
    for x in np.arange(acc.size):
        if  x != 0 :
            rx = acc[x]-acc[x-1]
            residual_matrix = np.append(residual_matrix, [rx])
        else:
            residual_matrix = np.append(residual_matrix, [rx])


        
    return residual_matrix

def fd(acc, dt): # funcion para derivar una matriz
    derivate_matrix = np.array([])
    last_x = 0
    for x in acc:
        dx = (x-last_x)/dt
        derivate_matrix = np.append(derivate_matrix, [dx])
        last_x = x
        
    return derivate_matrix

def fi(acc, dt):
    integral_matrix = np.array([])
    acc_sum = 0.0
    for x in acc:
        acc_sum = x*dt+acc_sum
        integral_matrix = np.append(integral_matrix, [acc_sum])
        
    return integral_matrix
    
def quaternion2RPY(quaternion):
    rpy = np.zeros([1, 3])
    qx = quaternion[0]
    qy = quaternion[1]
    qz = quaternion[2]
    qw = quaternion[3]
    # roll (x-axis rotation)
    sinr_cosp = +2.0 * (qw * qx + qy * qz)
    cosr_cosp = +1.0 - 2.0 * (qx * qx + qy * qy)
    roll = math.atan2(sinr_cosp, cosr_cosp)

    # pitch (y-axis rotation)
    sinp = +2.0 * (qw * qy - qz * qx)
    if math.fabs(sinp) >= 1:
        pitch = math.copysign(math.pi/ 2, sinp)# use 90 degrees if out of range
    else:
        pitch = math.asin(sinp)

    # yaw (z-axis rotation)
    siny_cosp = +2.0 * (qw* qz + qx*qy)
    cosy_cosp = +1.0 - 2.0 * (qy*qy + qz*qz)
    yaw = math.atan2(siny_cosp, cosy_cosp)
  
    rpy[0][0] = roll
    rpy[0][1] = pitch
    rpy[0][2] = yaw
    return rpy


def main():

    plot_est = [0, 0, 0, 1] # estimacion: pos, velocidad, aceleracion, orientacion
    plot_res = [0, 0, 0, 1] # residuales: pos, velocidad, aceleracion, orientacion
    plot_error = [0, 0, 0, 1]  #errores: pos, velocidad, aceleracion, orientacion
    plot_debug = [0, 0]      #debug residual de posicion proveniente de la velocidad


    estPosition = np.zeros([0, 3])
    gtPosition = np.zeros([0, 3])
    estVelocity = np.zeros([0, 3])
    gtVelocity = np.zeros([0, 3])
    estAcc = np.zeros([0, 3])
    gtAcc= np.zeros([0, 3])
    estOrientationQ = np.zeros([0, 4])
    gtOrientationQ = np.zeros([0, 4])
    estOrientationRPY = np.zeros([0, 3])
    gtOrientationRPY = np.zeros([0, 3])
    Fs = 20 # Frecuencia de la camara
    #Fs = 0.5*10**-3
    Ts = 1.0/Fs # intervalo de tiempo
   
    #debug
    estAngVelocity= np.zeros([0, 3])

    maxTime = 50.0

    with open('/home/lujano/Documents/outputVISlam.csv', newline='') as csvfile:
        reader = csv.reader(csvfile)
        for row in reader:
            estPosition = np.append( estPosition, [[float(row[0]), float(row[1]), float(row[2])]], 0)
            estVelocity = np.append( estVelocity, [[float(row[3]), float(row[4]), float(row[5])]], 0)
            estAcc = np.append( estAcc, [[float(row[6]), float(row[7]), float(row[8])]], 0)
            estOrientationQ = np.append( estOrientationQ, [[float(row[9]), float(row[10]), float(row[11]), float(row[12])]], 0)
            gtPosition = np.append( gtPosition, [[float(row[13]), float(row[14]), float(row[15])]], 0)
            gtVelocity = np.append( gtVelocity, [[float(row[16]), float(row[17]), float(row[18])]], 0)
            gtOrientationQ = np.append( gtOrientationQ, [[float(row[19]), float(row[20]), float(row[21]), float(row[22])]], 0)
            #estAngVelocity = np.append( estAngVelocity, [[float(row[23]), float(row[24]), float(row[25])]], 0)

        
    for quaternion in  estOrientationQ :
        estOrientationRPY = np.append( estOrientationRPY, quaternion2RPY(quaternion), 0)
    for quaternion in  gtOrientationQ :
        gtOrientationRPY = np.append( gtOrientationRPY, quaternion2RPY(quaternion), 0)
   
    
    rest = np.full_like(estPosition[:, 0], 1)
    estPosition[:, 0] = estPosition[:, 0] - estPosition[0, 0]*rest 
    estPosition[:, 1] = estPosition[:, 1] - estPosition[0, 1]*rest 
    estPosition[:, 2] = estPosition[:, 2] - estPosition[0, 2]*rest 

    estPosition[:, 0] = estPosition[:, 0]*5.0 + gtPosition[0, 0]*rest 
    estPosition[:, 1] = estPosition[:, 1]*5.0 + gtPosition[0, 1]*rest 
    estPosition[:, 2] = estPosition[:, 2]*5.0 + gtPosition[0, 2]*rest 
    
    gtAccx= fd(gtVelocity[:, 0], 1/20.0)
    gtAccy = fd(gtVelocity[:, 1], 1/20.0)
    gtAccz = fd(gtVelocity[:, 2], 1/20.0)
            
    time = np.arange(0, Ts*(estPosition[:, 0].size), Ts )


    
    if plot_est[0] == 1:
        # Plot position
        plt.figure()

        plt.subplot(3, 1, 1)
        plt.plot(time, estPosition[:, 0], 'b-', linewidth=2, label='Posicion x estimada')
        plt.plot(time, gtPosition[:, 0], 'r-', linewidth=2, label='Posicion x gt')
        plt.ylabel("x(m)")
        plt.xlabel("t(s)")
        plt.legend()
        

        plt.subplot(3, 1, 2)
        plt.plot(time, estPosition[:, 1], 'b-', linewidth=2, label='Posicion y estimada')
        plt.plot(time, gtPosition[:, 1], 'r-', linewidth=2, label='Posicion y gt')
        plt.ylabel("y(m)")
        plt.xlabel("t(s)")
        plt.legend()
        
        plt.subplot(3, 1, 3)
        plt.plot(time, estPosition[:, 2], 'b-', linewidth=2, label='Posicion z estimada')
        plt.plot(time, gtPosition[:, 2], 'r-', linewidth=2, label='Posicion z gt')
        plt.ylabel("z(m)")
        plt.xlabel("t(s)")
        plt.legend()

    if plot_est[1] == 1:
        # Plot velocity
        plt.figure()

        plt.subplot(3, 1, 1)
        plt.plot(time, estVelocity[:, 0], 'b-', linewidth=2, label='Velocidad en x estimada')
        plt.plot(time, gtVelocity[:, 0], 'r-', linewidth=2, label='Velocidad en x gt')
        plt.ylabel("Vx(m/s)")
        plt.xlabel("t(s)")
        plt.legend()

        plt.subplot(3, 1, 2)
        plt.plot(time, estVelocity[:, 1], 'b-', linewidth=2, label='Velocidad en y estimada')
        plt.plot(time, gtVelocity[:, 1], 'r-', linewidth=2, label='Velocidad en y gt')
        plt.ylabel("Vy(m/s)")
        plt.xlabel("t(s)")
        plt.legend()
        
        plt.subplot(3, 1, 3)
        plt.plot(time, estVelocity[:, 2], 'b-', linewidth=2, label='Velocidad en z estimada')
        plt.plot(time, gtVelocity[:, 2], 'r-', linewidth=2, label='Velocidad en z gt')
        plt.ylabel("Vz(m/s)")
        plt.xlabel("t(s)")
        plt.legend()

    if plot_est[2] == 1:
        # Plot acceleration
        plt.figure()

        plt.subplot(3, 1, 1)
        plt.plot(time, estAcc[:, 0], 'b-', linewidth=2, label='Aceleración en x estimada')
        plt.plot(time, gtAccx, 'r-', linewidth=2, label='Aceleración en x gt')
        plt.ylabel("Ax(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()

        plt.subplot(3, 1, 2)
        plt.plot(time, estAcc[:, 1], 'b-', linewidth=2, label='Aceleración en y estimada')
        plt.plot(time, gtAccy, 'r-', linewidth=2, label='Aceleración en y gt')
        plt.ylabel("Ay(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()
        
        plt.subplot(3, 1, 3)
        plt.plot(time, estAcc[:, 2], 'b-', linewidth=2, label='Aceleración en z estimada')
        plt.plot(time, gtAccz, 'r-', linewidth=2, label='Aceleración en z gt')
        plt.ylabel("Az(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()

    if plot_est[3] == 1:
        # Plot orientation (RPY)
        plt.figure()

        plt.subplot(3, 1, 1)
        plt.plot(time, remap(estOrientationRPY[:, 0]*180/math.pi), 'b-', linewidth=2, label='Roll estimado')
        plt.plot(time,  remap(gtOrientationRPY[:, 0]*180/math.pi), 'r-', linewidth=2, label='Roll gt')
        plt.ylabel("roll(°)")
        plt.xlabel("t(s)")
        plt.legend()
        plt.minorticks_on()
        plt.grid(b=True, which='major', color=[0.3, 0.3, 0.3], linestyle='-')
        plt.grid(b=True, which='minor', color=[0.6, 0.3, 0.3], linestyle='--')

        plt.subplot(3, 1, 2)
        plt.plot(time, estOrientationRPY[:, 2]*180/math.pi, 'b-', linewidth=2, label='Pitch estimado')
        plt.plot(time, gtOrientationRPY[:, 2]*180/math.pi, 'r-', linewidth=2, label='Pitch gt')
        plt.ylabel("pitch(°)")
        plt.xlabel("t(s)")
        plt.legend()
        plt.minorticks_on()
        plt.grid(b=True, which='major', color=[0.3, 0.3, 0.3], linestyle='-')
        plt.grid(b=True, which='minor', color=[0.6, 0.3, 0.3], linestyle='--')

        
        plt.subplot(3, 1, 3)
        plt.plot(time, estOrientationRPY[:, 2]*180/math.pi, 'b-', linewidth=2, label='Yaw estimado')
        plt.plot(time, gtOrientationRPY[:, 2]*180/math.pi, 'r-', linewidth=2, label='Yaw gt')
        plt.ylabel("yaw(°)")
        plt.xlabel("t(s)")
        plt.legend()
        plt.minorticks_on()
        plt.grid(b=True, which='major', color=[0.3, 0.3, 0.3], linestyle='-')
        plt.grid(b=True, which='minor', color=[0.6, 0.3, 0.3], linestyle='--')


       
    if plot_res[0] == 1:
         # Plot residual position
        plt.figure()

        plt.subplot(3, 1, 1)
        plt.plot(time, residual(estPosition[:, 0]), 'b-', linewidth=2, label='Residual Posicion x estimada')
        plt.plot(time, residual(gtPosition[:, 0]), 'r-', linewidth=2, label='Residual Posicion x gt')
        plt.ylabel("Rx(m)")
        plt.xlabel("t(s)")
        plt.legend()

        plt.subplot(3, 1, 2)
        plt.plot(time, residual(estPosition[:, 1]), 'b-', linewidth=2, label='Posicion y estimada')
        plt.plot(time, residual(gtPosition[:, 1]), 'r-', linewidth=2, label='Posicion y gt')
        plt.ylabel("Ry(m)")
        plt.xlabel("t(s)")
        plt.legend()
        
        plt.subplot(3, 1, 3)
        plt.plot(time, residual(estPosition[:, 2]), 'b-', linewidth=2, label='Residual Posicion z estimada')
        plt.plot(time, residual(gtPosition[:, 2]), 'r-', linewidth=2, label='Residual Posicion z gt')
        plt.ylabel("Rz(m)")
        plt.xlabel("t(s)")
        plt.legend()

    if plot_res[1] == 1:
        # Plot residual velocity
        plt.figure()

        plt.subplot(3, 1, 1)
        plt.plot(time, residual(estVelocity[:, 0]), 'b-', linewidth=2, label='Residual Velocidad en x estimada')
        plt.plot(time, residual(gtVelocity[:, 0]), 'r-', linewidth=2, label='Residual Velocidad en x gt')
        plt.ylabel("RVx(m/s)")
        plt.xlabel("t(s)")
        plt.legend()

        plt.subplot(3, 1, 2)
        plt.plot(time, residual(estVelocity[:, 1]), 'b-', linewidth=2, label='Residual Velocidad en y estimada')
        plt.plot(time, residual(gtVelocity[:, 1]), 'r-', linewidth=2, label='Residual Velocidad en y gt')
        plt.ylabel("RVy(m/s)")
        plt.xlabel("t(s)")
        plt.legend()
        
        plt.subplot(3, 1, 3)
        plt.plot(time, residual(estVelocity[:, 2]), 'b-', linewidth=2, label='Residual Velocidad en z estimada')
        plt.plot(time, residual(gtVelocity[:, 2]), 'r-', linewidth=2, label='ResidualVelocidad en z gt')
        plt.ylabel("RVz(m/s)")
        plt.xlabel("t(s)")
        plt.legend()

    if plot_res[2] == 1:
        # Plot residual acceleration
        plt.figure()

        plt.subplot(3, 1, 1)
        plt.plot(time, residual(estAcc[:, 0]), 'b-', linewidth=2, label=' Residual Aceleración en x estimada')
        plt.plot(time, residual(gtAccx), 'r-', linewidth=2, label='Residual Aceleración en x gt')
        plt.ylabel("RAx(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()

        plt.subplot(3, 1, 2)
        plt.plot(time, residual(estAcc[:, 1]), 'b-', linewidth=2, label='Residual Aceleración en y estimada')
        plt.plot(time, residual(gtAccy), 'r-', linewidth=2, label='Residual Aceleración en y gt')
        plt.ylabel("RAy(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()
        
        plt.subplot(3, 1, 3)
        plt.plot(time, residual(estAcc[:, 2]), 'b-', linewidth=2, label='Residual Aceleración en z estimada')
        plt.plot(time, residual(gtAccz), 'r-', linewidth=2, label='Residual Aceleración en z gt')
        plt.ylabel("RAz(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()

    if plot_res[3] == 1:
        # Plot residual orientation (RPY)
        plt.figure()

        plt.subplot(3, 1, 1)
        plt.plot(time, error(residual(estOrientationRPY[:, 0]*180/math.pi)), 'b-', linewidth=2, label='Residual Roll estimado')
        plt.plot(time, error(residual(gtOrientationRPY[:, 0]*180/math.pi)), 'r-', linewidth=2, label='Residual Roll gt')
        plt.ylabel("Rroll(°)")
        plt.xlabel("t(s)")
        plt.legend()
        plt.xlim([0, maxTime])
        plt.minorticks_on()
        plt.grid(b=True, which='major', color=[0.3, 0.3, 0.3], linestyle='-')
        plt.grid(b=True, which='minor', color=[0.6, 0.3, 0.3], linestyle='--')


        plt.subplot(3, 1, 2)
        plt.plot(time, residual(estOrientationRPY[:, 1]*180/math.pi), 'b-', linewidth=2, label='Residual Pitch estimado')
        plt.plot(time, residual(gtOrientationRPY[:, 1]*180/math.pi), 'r-', linewidth=2, label='Residual Pitch gt')
        plt.ylabel("Rpitch(°)")
        plt.xlabel("t(s)")
        plt.xlim([0, maxTime])
        plt.legend()
        plt.minorticks_on()
        plt.grid(b=True, which='major', color=[0.3, 0.3, 0.3], linestyle='-')
        plt.grid(b=True, which='minor', color=[0.6, 0.3, 0.3], linestyle='--')

        
        plt.subplot(3, 1, 3)
        plt.plot(time, error(residual(estOrientationRPY[:, 2]*180/math.pi)), 'b-', linewidth=2, label='Residual Yaw estimado')
        plt.plot(time, error(residual(gtOrientationRPY[:, 2]*180/math.pi)), 'r-', linewidth=2, label='Residual Yaw gt')
        plt.ylabel("Ryaw(°)")
        plt.xlabel("t(s)")
        plt.xlim([0, maxTime])
        plt.minorticks_on()
        plt.grid(b=True, which='major', color=[0.3, 0.3, 0.3], linestyle='-')
        plt.grid(b=True, which='minor', color=[0.6, 0.3, 0.3], linestyle='--')

        plt.subplots_adjust(left=0.05, bottom=0.05, right=0.98, top=0.98, wspace=0.2, hspace=0.2)


    # plot errors
    if plot_error[0] == 1:
        # Plot position
        plt.figure()

        plt.subplot(3, 1, 1)
        plt.plot(time, gtPosition[:, 0]-estPosition[:, 0], 'b-', linewidth=2, label='Error Posicion x estimada ')
        plt.ylabel("Ex(m)")
        plt.xlabel("t(s)")
        plt.legend()

        plt.subplot(3, 1, 2)
        plt.plot(time, gtPosition[:, 1]- estPosition[:, 1], 'b-', linewidth=2, label='Error Posicion y estimada')
        plt.ylabel("Ey(m)")
        plt.xlabel("t(s)")
        plt.legend()
        
        plt.subplot(3, 1, 3)
        plt.plot(time, gtPosition[:, 2]-estPosition[:, 2], 'b-', linewidth=2, label='Error Posicion z estimada')
        plt.ylabel("Ez(m)")
        plt.xlabel("t(s)")
        plt.legend()

    if plot_error[1] == 1:
        # Plot velocity
        plt.figure()

        plt.subplot(3, 1, 1)
        plt.plot(time, gtVelocity[:, 0]- estVelocity[:, 0], 'b-', linewidth=2, label='Error Velocidad en x estimada')
        plt.ylabel("EVx(m/s)")
        plt.xlabel("t(s)")
        plt.legend()

        plt.subplot(3, 1, 2)
        plt.plot(time, gtVelocity[:, 1]-estVelocity[:, 1], 'b-', linewidth=2, label='Error Velocidad en y estimada')
        plt.ylabel("EVy(m/s)")
        plt.xlabel("t(s)")
        plt.legend()
        
        plt.subplot(3, 1, 3)
        plt.plot(time, gtVelocity[:, 2]-estVelocity[:, 2], 'b-', linewidth=2, label='Error Velocidad en z estimada')
        plt.ylabel("EVz(m/s)")
        plt.xlabel("t(s)")
        plt.legend()

    if plot_error[2] == 1:
        # Plot acceleration
        plt.figure()

        plt.subplot(3, 1, 1)
        plt.plot(time, gtAccx- estAcc[:, 0], 'b-', linewidth=2, label='Error Aceleración en x estimada')
        plt.ylabel("EAx(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()

        plt.subplot(3, 1, 2)
        plt.plot(time, gtAccy- estAcc[:, 1], 'b-', linewidth=2, label='Error Aceleración en y estimada')
        plt.ylabel("EAy(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()
        
        plt.subplot(3, 1, 3)
        plt.plot(time, gtAccz- estAcc[:, 2], 'b-', linewidth=2, label='Error Aceleración en z estimada')
        plt.ylabel("EAz(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()


    if plot_error[3] == 1:
        # Plot error orientation (RPY)
        plt.figure()


        plt.subplot(3, 1, 1)
        plt.plot(time,error((gtOrientationRPY[:, 0]-estOrientationRPY[:, 0])*180/math.pi), 'b-', linewidth=2, label='Error Roll estimado')
        plt.ylabel("Eroll(°)")
        plt.xlabel("t(s)")
        plt.legend()
        plt.minorticks_on()
        plt.grid(b=True, which='major', color=[0.3, 0.3, 0.3], linestyle='-')
        plt.grid(b=True, which='minor', color=[0.6, 0.3, 0.3], linestyle='--')


        plt.subplot(3, 1, 2)
        plt.plot(time, error((gtOrientationRPY[:, 1]-estOrientationRPY[:, 1])*180/math.pi), 'b-', linewidth=2, label='Error Pitch estimado')
        plt.ylabel("Epitch(°)")
        plt.xlabel("t(s)")
        plt.legend()
        plt.minorticks_on()
        plt.grid(b=True, which='major', color=[0.3, 0.3, 0.3], linestyle='-')
        plt.grid(b=True, which='minor', color=[0.6, 0.3, 0.3], linestyle='--')

        
        plt.subplot(3, 1, 3)
    
       
        plt.plot(time, error((gtOrientationRPY[:, 2]-estOrientationRPY[:, 2])*180/math.pi), 'b-', linewidth=2, label='Error Yaw estimado')
        plt.ylabel("Eyaw(°)")
        plt.xlabel("t(s)")
        plt.legend()
        plt.minorticks_on()
        plt.grid(b=True, which='major', color=[0.3, 0.3, 0.3], linestyle='-')
        plt.grid(b=True, which='minor', color=[0.6, 0.3, 0.3], linestyle='--')

    
    """
    if plot_error[3] == 1:
        # Plot velocidad angular
        plt.figure()

        plt.subplot(3, 1, 1)
        plt.plot(time, estAngVelocity[:, 0], 'b-', linewidth=2, label='Velocidad angular x estimada ')
        plt.ylabel("wx(m)")
        plt.xlabel("t(s)")
        plt.legend()

        plt.subplot(3, 1, 2)
        plt.plot(time, estAngVelocity[:, 1], 'b-', linewidth=2, label='Velocidad angular y estimada ')
        plt.ylabel("wy(m)")
        plt.xlabel("t(s)")
        plt.legend()
        
        plt.subplot(3, 1, 3)
        plt.plot(time, estAngVelocity[:, 2], 'b-', linewidth=2, label='Velocidad angular z estimada ')
        plt.ylabel("wz(m)")
        plt.xlabel("t(s)")
        plt.legend()
    """



    if plot_debug[0] == 1:
        # Plot residual de posicion proveniente de la integracion de la velocidad de la imu
        plt.figure()

        plt.subplot(3, 1, 1)
        plt.plot(time, residual(fi(estVelocity[:, 0], 0.05)), 'b-', linewidth=2, label=' Residual de Pose en x estimada')
        plt.plot(time, residual(gtPosition[:,0]), 'r-', linewidth=2, label='Residual de Pose en x gt')
        plt.ylabel("Rx(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()

        plt.subplot(3, 1, 2)
        plt.plot(time, residual(fi(estVelocity[:, 1], 0.05)), 'b-', linewidth=2, label=' Residual de Pose en y estimada')
        plt.plot(time, residual(gtPosition[:,1]), 'r-', linewidth=2, label='Residual de Pose en y gt')
        plt.ylabel("Ry(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()
        
        plt.subplot(3, 1, 3)
        plt.plot(time, residual(fi(estVelocity[:, 2], 0.05)), 'b-', linewidth=2, label=' Residual de Pose en z estimada')
        plt.plot(time, residual(gtPosition[:,2]), 'r-', linewidth=2, label='Residual de Pose en z gt')
        plt.ylabel("Rz(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()


    if plot_debug[1] == 1:
        # Plot residual de posicion proveniente de la integracion de la velocidad de la imu
        plt.figure()

        positionx = fi(estVelocity[:, 0], 0.05)
        positiony = fi(estVelocity[:, 1], 0.05)
        positionz = fi(estVelocity[:, 2], 0.05)

        
        positionx = positionx - positionx[0]*rest +gtPosition[0,0]*rest
        positiony = positiony - positiony[0]*rest +gtPosition[0,1]*rest
        positionz = positionz - positionz[0]*rest +gtPosition[0,2]*rest

        plt.subplot(3, 1, 1)
        plt.plot(time, positionx , 'b-', linewidth=2, label='Pose integrada x estimada')
        plt.plot(time, gtPosition[:,0], 'r-', linewidth=2, label='Pose en x gt')
        plt.ylabel("Rx(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()

        plt.subplot(3, 1, 2)
        plt.plot(time, positiony, 'b-', linewidth=2, label=' Pose integrada y estimada')
        plt.plot(time, gtPosition[:,1], 'r-', linewidth=2, label='Pose en y gt')
        plt.ylabel("Ry(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()
        
        plt.subplot(3, 1, 3)
        plt.plot(time, positionz, 'b-', linewidth=2, label=' Pose integrada z estimada')
        plt.plot(time, gtPosition[:,2], 'r-', linewidth=2, label='Pose en z gt')
        plt.ylabel("Rz(m/s²)")
        plt.xlabel("t(s)")
        plt.legend()



    plt.show()
    



if __name__ == "__main__": main()

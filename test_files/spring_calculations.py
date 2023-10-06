import math

radius = 2.5 # distance from the pivot of my joint to the placement of my spring (in)
dorsi = 20 # number of degrees being rotated (deg)
plantar = 10 # number of degrees being rotated (deg)

F_heel_low = 14.5515 # lb-in/deg
F_heel_high = 31.327 # lb-in/deg
F_foot_low = 60.90906 # lb-in/deg
F_foot_high = 215.6006 # lb-in/deg

# print("K calculations:")
dx = (2*math.pi*radius*plantar)/360 # (in)
low_heel_goal_k = (F_heel_low*plantar)/dx
high_heel_goal_k = (F_heel_high*plantar)/dx
dx = (2*math.pi*radius*dorsi)/360 # (in)
low_foot_goal_k = (F_foot_low*dorsi)/dx
high_foot_goal_k = (F_foot_high*dorsi)/dx
# print(f"K heel for {plantar} plantarflexion:\n {low_heel_goal_k}-{high_heel_goal_k}")
# print(f"K foot for {dorsi} dorsiflexion:\n {low_foot_goal_k}-{high_foot_goal_k}")
# print("\n")

### Modulus of for different materials ###
# music wire (steel):       30*10^6 psi
# chrome-silicon steel:     30*10^6 psi
### COMMON WIRE DIAMETERS ###
#   .049
#   .055
#   .072
#   .08
#   .091
#   .105
#   .112
#   .12
#   .125 (this is about what the big spring Major has is)
#   .135
#   .148
#   .192
### COMMON COIL DIAMETERS ###
#   .975
#   1.0
#   1.031
#   1.075
#   1.219
#   1.225 (this is about what the big spring Major has is)
#   1.25
#   1.281
#   1.375
#   1.46
#   1.687
#   1.937

G = 13*(10**6) # modulus of music wire (pa)
d = .112        # diameter of wire (in)
N = 2.1           # number of active coils
D = 1.031      # coil dimater (in)

k = (G*(d**4))/(8*N*( D**3))
print("Value of k: ",k)
print(f"K parameters: \n wire diameter: {d} \n coil diameter: {D} \n active coils: {N}")

# print(f"dorsi N: {(G*(d**4))/(8*low_foot_goal_k*(D**3))}, {(G*(d**4))/(8*high_foot_goal_k*(D**3))}")
# print(f"plantar N: {(G*(d**4))/(8*low_heel_goal_k*(D**3))}, {(G*(d**4))/(8*high_heel_goal_k*(D**3))}")

Nt = N + 2
Lf = 2.0
p = (Lf - 2*d)/N
# Lf = p*N + 2*d
Ls = d*Nt


# S = .1 # solid height

# L_low =((F_low/k)-S)/(N+D)
# L_high =((F_high/k)-S)/(N+D)
NLOW = 2
NHIGH = 20
# print(f"length of spring: {L_low}-{L_high}")
radii = [4, 3.5, 3, 2.5, 2]
coil_diameters = [.4, .45, .5, .55, .6, 65,.7, .75, .938,.97,.975,1.0,1.016,1.031,1.075,1.219,1.225,1.25,1.281,1.375,1.46,1.687,1.937]
wire_diameters = [.049,.055,.072,.08,.085,.091,.092,.095,.096,.098,.1,.105,.112,.12,.125,.135,.148,.163, .177,.187,.192,.207,.25,.312]
# materials: Beryllium Copper UNS C17200, Phosphor Bronze – Copper Alloy UNS C51000, 
# materials = 
# tensile =  [965, 131] # in MPa
# poisson = [.3, .34] 

for rad in radii:
    dx = (2*math.pi*rad*plantar)/360 # (in)
    print(f"dx for plantarflexion {dx}")
    low_heel_goal_k = ((F_heel_low*plantar)/rad)/dx
    high_heel_goal_k = ((F_heel_high*plantar)/rad)/dx
    dx = (2*math.pi*rad*dorsi)/360 # (in)
    print(f"dx for dorsiflexion {dx}")
    low_foot_goal_k = ((F_foot_low*dorsi)/rad)/dx
    high_foot_goal_k = ((F_foot_high*dorsi)/rad)/dx
    print(f"at a radius of {rad} \n")
    print(f"K heel for {plantar} plantarflexion:\n {low_heel_goal_k}-{high_heel_goal_k}")
    print(f"K foot for {dorsi} dorsiflexion:\n {low_foot_goal_k}-{high_foot_goal_k}")
    print("\n")
    for coils in coil_diameters:
        for wires in wire_diameters:    
            dorsi_l =(G*(wires**4))/(8*low_foot_goal_k*(coils**3))
            dorsi_h =(G*(wires**4))/(8*high_foot_goal_k*(coils**3))
            plantar_l = (G*(wires**4))/(8*low_heel_goal_k*(coils**3))
            plantar_h = (G*(wires**4))/(8*high_heel_goal_k*(coils**3))
            if dorsi_l > NLOW and dorsi_h < NHIGH and dorsi_h > NLOW and dorsi_l < NHIGH and (coils/wires) >= 4:
                print(f"coil diameter: {coils}, wire diameter: {wires}")
                print(f"dorsi N: {(G*(wires**4))/(8*low_foot_goal_k*(coils**3))}, {(G*(wires**4))/(8*high_foot_goal_k*(coils**3))}")
                print(f"difference: {dorsi_l - dorsi_h}")
                print(f"spring Index: {coils/wires} \n")
            # if plantar_l > NLOW and plantar_h < NHIGH and plantar_l < NHIGH and plantar_h > NLOW:
            #     print(f"coil diameter: {coils}, wire diameter: {wires}")
            #     print(f"plantar N: {(G*(wires**4))/(8*low_heel_goal_k*(coils**3))}, {(G*(wires**4))/(8*high_heel_goal_k*(coils**3))}")
            #     print(f"spring Index: {coils/wires} \n")
    print("\n\n")
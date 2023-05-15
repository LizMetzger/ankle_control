import math

radius = 3 # distance from the pivot of my joint to the placement of my spring (in)
dorsi = 20 # number of degrees being rotated (deg)
plantar = 10 # number of degrees being rotated (deg)

F_heel_low = 14.5515 # lb-in/deg
F_heel_high = 31.327 # lb-in/deg
F_foot_low = 60.90906 # lb-in/deg
F_foot_high = 215.6006 # lb-in/deg

print("K calculations:")
dx = (2*math.pi*radius*plantar)/360 # (in)
low_heel_goal_k = F_heel_low/dx
high_heel_goal_k = F_heel_high/dx
dx = (2*math.pi*radius*dorsi)/360 # (in)
low_foot_goal_k = F_foot_low/dx
high_foot_goal_k = F_foot_high/dx
print(f"K heel for {plantar} plantarflexion:\n {low_heel_goal_k}-{high_heel_goal_k}")
print(f"K foot for {dorsi} dorsiflexion:\n {low_foot_goal_k}-{high_foot_goal_k}")
print("\n")

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
#   .125 (this is about what the big spring Major has is)
#   .135
#   .148
#   .192
### COMMON COIL DIAMETERS ###
#   .975
#   1.0
#   1.225 (this is about what the big spring Major has is)
#   1.46
#   1.687
#   1.937

G = 30*(10**6) # modulus of music wire (psi)
d = .125       # diameter of wire (in)
N = 8          # number of active coils
D = 1.225      # coil dimater (in)

k = (G*(d**4))/(8*N*(D**3))
print("Value of k: ",k)
print(f"K parameters: \n wire diameter: {d} \n coil diameter: {D} \n active coils: {N}")

Nt = N + 2
Lf = 2.0
p = (Lf - 2*d)/N
# Lf = p*N + 2*d
Ls = d*Nt


# S = .1 # solid height

# L_low =((F_low/k)-S)/(N+D)
# L_high =((F_high/k)-S)/(N+D)

# print(f"length of spring: {L_low}-{L_high}")
l_t = int(input("totalDataLength:"))
l_v = int(input("validDataLength:"))
u_v = int(input("validUnitCount:"))
u_c = int(input("compressUnitCount:"))
u_t = int(input("totalUnitCount:"))

r_c = 1.0 - (l_t - l_v) / (u_c * 4096)
print("r_c = ", r_c)

f_c = 1.0 - (u_t - u_v) / u_c
print("f_c = ", f_c)

r_f = 1.0 - r_c/f_c
print("r_f = ", r_f)
# vertices = []
# edges = []

with open("softbunny.1.node", 'r') as vertex_file, \
    open("softbunny.1.edge", 'r') as edge_file,\
    open("softbunny.1.face", 'r') as face_file,\
    open("softbunny.1.ele", 'r') as tetra_file,\
    open("softbunny.1.neigh", 'r') as neigh_file,\
    open("softbunny.tetra", 'w') as output:
    header = vertex_file.readline().split()
    vertex_count = int(header[0])
    header = edge_file.readline().split()
    edge_count = int(header[0])
    header = face_file.readline().split()
    face_count = int(header[0])
    header = tetra_file.readline().split()
    tetra_count = int(header[0])
    header = neigh_file.readline().split()
    neigh_count = int(header[0])
    output.write("vc " + str(vertex_count) + "\n")
    output.write("ec " + str(edge_count) + "\n")
    output.write("fc " + str(face_count) + "\n")
    output.write("tc " + str(tetra_count) + "\n")
    output.write("tnc " + str(neigh_count) + "\n\n")
    cnt = 0
    while (cnt < vertex_count):
        line = vertex_file.readline().split()
        if (not line): break
        vertex = (float(line[1]), float(line[2]), float(line[3]))
        output.write("v " + str(vertex).replace(",", "")[1:-1] + "\n")
        cnt += 1

    cnt = 0
    while (cnt < edge_count):
        line = edge_file.readline().split()
        if (not line): break
        edge = (int(line[1]), int(line[2]))
        output.write("e " + str(edge).replace(",", "")[1:-1] + "\n")
        cnt += 1

    cnt = 0
    while (cnt < face_count):
        line = face_file.readline().split()
        if (not line): break
        face = (int(line[1]), int(line[2]), int(line[3]))
        output.write("f " + str(face).replace(",", "")[1:-1] + "\n")
        cnt += 1

    cnt = 0
    while (cnt < tetra_count):
        line = tetra_file.readline().split()
        if (not line): break
        tetra = (int(line[1]), int(line[2]), int(line[3]), int(line[4]))
        output.write("t " + str(tetra).replace(",", "")[1:-1] + "\n")
        cnt += 1

    cnt = 0
    while (cnt < neigh_count):
        line = neigh_file.readline().split()
        if (not line): break
        neigh = (int(line[1]), int(line[2]), int(line[3]), int(line[4]))
        output.write("tn " + str(neigh).replace(",", "")[1:-1] + "\n")
        cnt += 1

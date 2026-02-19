import numpy as np
import matplotlib.pyplot as plt

# ===== PARAMÈTRES =====
size = 10
xmin, xmax = size * -1, size
ymin, ymax = size * -1, size
resolution = 500
nb_niveaux = 30
# ======================

# Grille complexe
x = np.linspace(xmin, xmax, resolution)
y = np.linspace(ymin, ymax, resolution)
X, Y = np.meshgrid(x, y)
Z = X + 1j * Y

# Fonction complexe (modifiable)
F = np.sin(Z**2) - 1

# Contours de la partie réelle
plt.figure()
contours = plt.contour(X, Y, np.real(F), levels=nb_niveaux)

plt.xlabel("Partie réelle")
plt.ylabel("Partie imaginaire")
plt.title("Contours de Re(f(z))")

plt.colorbar(contours, label="Re(f(z))")

plt.show()

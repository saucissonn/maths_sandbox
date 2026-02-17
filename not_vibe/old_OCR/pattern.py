import pygame
import json
import math
import sys
import random

background_colour = (255,255,255)
(width, height) = (812, 662)
screen = pygame.display.set_mode((width, height))
pygame.display.set_caption('Pattern')
screen.fill(background_colour)
pygame.display.flip()
FPS = 3000

class Grid:
    def __init__(self, size):
        self.size = size      # Put size at a power of 2 (easier)
        self.grid = [[0 for _ in range(self.size)] for _ in range(self.size)]
        self.brush = 5
        self.check_division_len = 32
        self.check_division = self.check_division_len*self.check_division_len
        with open("data.json", 'r') as file:
            self.data = json.load(file)
    
    def reset(self):
        self.grid = [[0 for _ in range(self.size)] for _ in range(self.size)]
    
    def write(self, v):
        if v in self.data:
            self.data[v].append(self.reduce())
        with open("data.json", 'w') as file:
            json.dump(self.data, file, indent=4)
    
    def print_grid(self):
        for i in range(self.size):
            print(self.grid[i])
    
    def average(self, x, y, l):
        res = 0
        for i in range(l):
            for j in range(l):
                if not (((y + i) < 0) or ((y + i) > grid.size) or ((x + j) < 0) or ((x + j) > grid.size)):
                    res += self.grid[y + i][x + j]
        return res/(l*l)

    def reduce(self):
        box = self.bbox()
        box_size = max(box[2] - box[0], box[3] - box[1])
        delta = box_size/self.size
        s = box_size//self.check_division_len
        l = [0]*self.check_division
        c = 0
        for y in range(self.check_division_len):
            for x in range(self.check_division_len):
                l[c] = self.average(box[0] + x*s, box[1] + y*s, s)*delta
                c += 1
        return l
    
    def valid_zone(self, event):
        x, y = event.pos
        return ((x >= 0) and (y >= 0) 
                and (y + self.brush <= self.size - 1)
                and (x + self.brush <= self.size - 1))
    
    def modify(self, event):
        x, y = event.pos
        for i in range(self.brush):
            for j in range(self.brush):
                self.grid[y + i][x + j] = 1

    def draw(self, event):
        x, y = event.pos
        pygame.draw.rect(screen, (0,0,0), (x, y, self.brush, self.brush))
    
    def bbox(self):
        min_x, min_y = self.size - 1, self.size - 1
        max_x, max_y = 0, 0
        for y in range(self.size):
            for x in range(self.size): 
                value = self.grid[y][x]
                if value == 1:
                    if x < min_x:
                        min_x = x
                    if y < min_y:
                        min_y = y
                    if x > max_x:
                        max_x = x
                    if y > max_y:
                        max_y = y
        if ((max_x - min_x) > (max_y - min_y)):
            lmax = max_x - min_x - (max_y - min_y)
            min_y -= lmax//2
            max_y += lmax//2
        else:
            lmax = max_y - min_y - (max_x - min_x)
            min_x -= lmax//2
            max_x += lmax//2
        
        if ((max_x - min_x) < self.check_division_len):
            print((max_x - min_x))
            min_x -= self.check_division_len
            max_x += self.check_division_len
        if ((max_y - min_y) < self.check_division_len):
            min_y -= self.check_division_len
            max_y += self.check_division_len
            
        return min_x, min_y, max_x, max_y
    
    def average_json(self):
        res = [[0 for _ in range(self.check_division)] for _ in range(10)]
        for v in range(10):
            if str(v) in self.data:
                l = len(self.data[str(v)])
                for i in range(l):
                    for j in range(self.check_division):
                        res[v][j] += self.data[str(v)][i][j]
                if (l == 0):
                    l+=1
                for j in range(self.check_division):
                    res[v][j] /= l

        return res
    
    def ask(self):
        input = self.reduce()
        compare = self.average_json()
        res = [0 for _ in range(10)]
        for i in range(10):
            for j in range(self.check_division):
                res[i] += abs(compare[i][j] - input[j])
        
        imin = 0
        mini = res[0]
        for i in range(10):
            if res[i] < mini:
                imin = i
                mini = res[i]
        return imin

    def update(self, event):
        if self.valid_zone(event):
            self.modify(event)
            self.draw(event)

class Generative:
    def __init__(self, grid):
        self.original_grid = grid
        self.grid = grid.grid
        self.goal = 0
    
    def generate_goal(self):
        self.goal = random.randint(0,9)
        return self.goal
    
    
    

    

grid = Grid(512)
AI = Generative(grid)

running = True
clicked = False
clock = pygame.time.Clock()

pygame.font.init()
smallfont = pygame.font.SysFont('Arial',40)
text = smallfont.render('quit' , True , (0,0,0))

while running:
    clock.tick(FPS)
    mouse = pygame.mouse.get_pos()
    
    for event in pygame.event.get():
        
        if event.type == pygame.QUIT:
            running = False

        if event.type == pygame.MOUSEBUTTONDOWN:
            #print(event)
            clicked = True
            for btn in range(10):
                if width - 300 <= mouse[0] <= width and btn*(grid.size/10) <= mouse[1] <= (btn+1)*(grid.size/10):
                    screen.fill(background_colour)
                    grid.write(str(btn))
                    grid.reset()
            if 300 <= mouse[0] <= width-300 and 550 <= mouse[1] <= 650:
                #grid.reset()
                print("Chiffre trouvé: " + str(grid.ask()))
        
        if event.type == pygame.MOUSEBUTTONUP:
            clicked = False
        
        if clicked:
            grid.update(event)

    for btn in range(10):
        text = smallfont.render(str(btn) , True , (0,0,0))
        if width - 300 <= mouse[0] <= width and btn*(grid.size/10) <= mouse[1] <= (btn+1)*(grid.size/10):
            pygame.draw.rect(screen,(200, 200, 200),[width - 300, btn*(grid.size/10), 300, 50])
            
        else:
            pygame.draw.rect(screen,(100, 100, 100),[width - 300, btn*(grid.size/10), 300, 50])
        
        screen.blit(text , (width - 170, btn*(grid.size/10)))
    
    text = smallfont.render("CHECK" , True , (0,0,0))
    if 300 <= mouse[0] <= width-300 and 550 <= mouse[1] <= 650:
        pygame.draw.rect(screen,(200, 200, 200),[300, 550, width - 600, 100])
        
    else:
        pygame.draw.rect(screen,(100, 100, 100),[300, 550, width - 600, 100])

    screen.blit(text , (300, 550))

    pygame.display.update()
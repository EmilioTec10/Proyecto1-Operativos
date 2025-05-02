# gui.py
import pygame
import socket
import json
import sys

# Configuración de colores
COLORS = {
    "road": (50, 50, 50),
    "background": (200, 200, 200),
    "normal": (0, 0, 255),
    "sport": (255, 0, 0),
    "emergency": (0, 255, 0),
    "text": (0, 0, 0),
    "sign": (255, 255, 0)
}

# Configuración de comunicación
HOST = 'localhost'
PORT = 65432

class CarSimGUI:
    def __init__(self, width=800, height=600):
        pygame.init()
        self.screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption("Car Scheduling Simulator")
        self.clock = pygame.time.Clock()
        self.font = pygame.font.SysFont('Arial', 18)
        
        # Conectar al servidor C
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((HOST, PORT))
        
        # Estado inicial
        self.road_length = 100
        self.cars = []
        self.queues = {'left': [], 'right': []}
        self.current_sign = 'left'
        self.active_cars = []

    def draw_road(self):
        road_width = 50
        center_x = self.screen.get_width() // 2 - road_width // 2
        pygame.draw.rect(self.screen, COLORS["road"], 
                        (center_x, 0, road_width, self.screen.get_height()))
        
        # Dibujar letrero
        sign_text = self.font.render(f"SENTIDO: {self.current_sign.upper()}", 
                                   True, COLORS["text"], COLORS["sign"])
        self.screen.blit(sign_text, (self.screen.get_width()//2 - 60, 10))

    def draw_queues(self):
        queue_height = 30
        # Cola izquierda
        for i, car in enumerate(self.queues['left']):
            x = 50
            y = 50 + i * 40
            pygame.draw.rect(self.screen, COLORS[car['type']], 
                            (x, y, 30, queue_height))
        
        # Cola derecha
        for i, car in enumerate(self.queues['right']):
            x = self.screen.get_width() - 80
            y = 50 + i * 40
            pygame.draw.rect(self.screen, COLORS[car['type']], 
                            (x, y, 30, queue_height))

    def draw_moving_cars(self):
        for car in self.active_cars:
            pos_x = self.screen.get_width()//2 + car['position'] * 5
            pygame.draw.circle(self.screen, COLORS[car['type']], 
                              (pos_x, car['lane'] * 50 + 200), 15)

    def update_state(self):
        try:
            self.sock.sendall(b'get_state')
            data = self.sock.recv(4096).decode()
            state = json.loads(data)
            self.queues = state['queues']
            self.active_cars = state['active_cars']
            self.current_sign = state['current_sign']
        except Exception as e:
            print("Error updating state:", e)

    def handle_input(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.sock.send(b'exit')
                pygame.quit()
                sys.exit()
                
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_a:
                    self.sock.send(b'add_left:normal')
                elif event.key == pygame.K_s:
                    self.sock.send(b'add_left:sport')
                elif event.key == pygame.K_d:
                    self.sock.send(b'add_left:emergency')
                elif event.key == pygame.K_j:
                    self.sock.send(b'add_right:normal')
                elif event.key == pygame.K_k:
                    self.sock.send(b'add_right:sport')
                elif event.key == pygame.K_l:
                    self.sock.send(b'add_right:emergency')
                elif event.key == pygame.K_w:
                    self.sock.send(b'exit')
                    pygame.quit()
                    sys.exit()

    def run(self):
        while True:
            self.screen.fill(COLORS["background"])
            self.handle_input()
            self.update_state()
            
            self.draw_road()
            self.draw_queues()
            self.draw_moving_cars()
            
            pygame.display.flip()
            self.clock.tick(30)

if __name__ == "__main__":
    gui = CarSimGUI()
    gui.run()
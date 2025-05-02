import tkinter as tk
from tkinter import filedialog, messagebox
from config_reader import load_config
from cethreads_wrapper import run_simulation
import threading

class SimulatorGUI(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Simulación de Coches – Scheduling")
        self.geometry("800x200")

        # Botones
        tk.Button(self, text="Cargar config...", command=self.load).pack(pady=10)
        self.lbl = tk.Label(self, text="—")
        self.lbl.pack(pady=5)
        tk.Button(self, text="Ejecutar simulación", command=self.start).pack(pady=10)

        self.cfg = None

    def load(self):
        path = filedialog.askopenfilename(filetypes=[("JSON files","*.json")])
        if not path: return
        try:
            self.cfg = load_config(path)
            self.lbl.config(text=f"Cargado: {len(self.cfg['cars'])} coches")
        except Exception as e:
            messagebox.showerror("Error", str(e))

    def start(self):
        if not self.cfg:
            messagebox.showwarning("Atención", "Antes carga un config.json")
            return
        # ejecutamos en un hilo para no bloquear la GUI
        threading.Thread(target=self._run, daemon=True).start()

    def _run(self):
        self.lbl.config(text="Simulando…")
        try:
            run_simulation(self.cfg)
            self.lbl.config(text="Simulación finalizada")
        except Exception as e:
            messagebox.showerror("Error en simulación", str(e))

if __name__=="__main__":
    SimulatorGUI().mainloop()

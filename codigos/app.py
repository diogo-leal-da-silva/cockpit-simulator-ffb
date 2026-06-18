import json
import os
import subprocess
import threading
import tkinter as tk
from tkinter import ttk, messagebox, filedialog
from pathlib import Path

import sys

if getattr(sys, "frozen", False):
    APP_DIR = Path(sys.executable).resolve().parent
else:
    APP_DIR = Path(__file__).resolve().parent

CONFIG_FILE = APP_DIR / "config.json"

DEFAULT_CONFIG = {
    "arduino_cli": "arduino-cli",
    "default_fqbn": "arduino:avr:leonardo",
    "sketches": [
        {"game": "Euro Truck Simulator 2", "sketch": "sketches/ETS2/ETS2.ino", "fqbn": "arduino:avr:leonardo"},
        {"game": "DiRTRally 2.0", "sketch": "sketches/DiRTRally2/DiRTRally2.ino", "fqbn": "arduino:avr:leonardo"}
    ]
}


def load_config():
    if not CONFIG_FILE.exists():
        CONFIG_FILE.write_text(json.dumps(DEFAULT_CONFIG, indent=4, ensure_ascii=False), encoding="utf-8")
    return json.loads(CONFIG_FILE.read_text(encoding="utf-8"))


def save_config(config):
    CONFIG_FILE.write_text(json.dumps(config, indent=4, ensure_ascii=False), encoding="utf-8")


class ArduinoUploaderApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Arduino Game Sketch Uploader")
        self.geometry("760x520")
        self.minsize(720, 480)
        self.config_data = load_config()
        self.create_widgets()
        self.refresh_sketches()

    def create_widgets(self):
        main = ttk.Frame(self, padding=12)
        main.pack(fill="both", expand=True)

        title = ttk.Label(main, text="Enviar sketch para Arduino", font=("Segoe UI", 16, "bold"))
        title.pack(anchor="w")

        row1 = ttk.Frame(main)
        row1.pack(fill="x", pady=(14, 6))
        ttk.Label(row1, text="Jogo/sketch:").pack(side="left")
        self.sketch_combo = ttk.Combobox(row1, state="readonly")
        self.sketch_combo.pack(side="left", fill="x", expand=True, padx=8)

        row2 = ttk.Frame(main)
        row2.pack(fill="x", pady=6)
        ttk.Label(row2, text="Porta:").pack(side="left")
        self.port_entry = ttk.Entry(row2, width=18)
        self.port_entry.pack(side="left", padx=8)
        ttk.Button(row2, text="Detectar portas", command=self.detect_ports).pack(side="left")
        self.port_combo = ttk.Combobox(row2, width=18, state="readonly")
        self.port_combo.pack(side="left", padx=8)
        self.port_combo.bind("<<ComboboxSelected>>", lambda e: self.port_entry.delete(0, tk.END) or self.port_entry.insert(0, self.port_combo.get()))

        row3 = ttk.Frame(main)
        row3.pack(fill="x", pady=6)
        ttk.Label(row3, text="FQBN/placa:").pack(side="left")
        self.fqbn_entry = ttk.Entry(row3)
        self.fqbn_entry.pack(side="left", fill="x", expand=True, padx=8)
        self.fqbn_entry.insert(0, self.config_data.get("default_fqbn", "arduino:avr:leonardo"))

        row4 = ttk.Frame(main)
        row4.pack(fill="x", pady=6)
        ttk.Button(row4, text="Adicionar sketch", command=self.add_sketch).pack(side="left")
        ttk.Button(row4, text="Instalar/atualizar core AVR", command=lambda: self.run_async(self.install_avr_core)).pack(side="left", padx=8)
        ttk.Button(row4, text="Compilar", command=lambda: self.run_async(self.compile_selected)).pack(side="left", padx=8)
        ttk.Button(row4, text="Compilar e enviar", command=lambda: self.run_async(self.compile_upload_selected)).pack(side="left", padx=8)

        self.progress = ttk.Progressbar(main, mode="indeterminate")
        self.progress.pack(fill="x", pady=(10, 8))

        ttk.Label(main, text="Saída:").pack(anchor="w")
        self.log = tk.Text(main, height=16, wrap="word")
        self.log.pack(fill="both", expand=True)

    def refresh_sketches(self):
        self.sketch_combo["values"] = [s["game"] for s in self.config_data.get("sketches", [])]
        if self.sketch_combo["values"]:
            self.sketch_combo.current(0)
            self.on_sketch_changed()
        self.sketch_combo.bind("<<ComboboxSelected>>", lambda e: self.on_sketch_changed())

    def on_sketch_changed(self):
        item = self.get_selected_item()
        if item:
            self.fqbn_entry.delete(0, tk.END)
            self.fqbn_entry.insert(0, item.get("fqbn", self.config_data.get("default_fqbn", "arduino:avr:leonardo")))

    def get_selected_item(self):
        idx = self.sketch_combo.current()
        sketches = self.config_data.get("sketches", [])
        if idx < 0 or idx >= len(sketches):
            return None
        return sketches[idx]

    def resolve_sketch_folder(self, sketch_path):
        p = Path(sketch_path)
        if not p.is_absolute():
            p = APP_DIR / p
        if p.suffix.lower() == ".ino":
            return p.parent
        return p

    def cli(self):
        return self.config_data.get("arduino_cli", "arduino-cli")

    def append_log(self, text):
        self.log.insert(tk.END, text + "\n")
        self.log.see(tk.END)
        self.update_idletasks()

    def run_command(self, args):
        self.append_log("> " + " ".join(str(a) for a in args))
        try:
            process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding="utf-8", errors="replace")
            for line in process.stdout:
                self.append_log(line.rstrip())
            code = process.wait()
            if code != 0:
                raise RuntimeError(f"Comando terminou com erro {code}.")
            return code
        except FileNotFoundError:
            raise RuntimeError("Não achei o arduino-cli. Instale ou coloque o arduino-cli.exe na pasta do programa.")

    def run_async(self, func):
        def target():
            self.progress.start(10)
            try:
                func()
                self.append_log("✅ Finalizado.")
            except Exception as e:
                self.append_log(f"❌ Erro: {e}")
                messagebox.showerror("Erro", str(e))
            finally:
                self.progress.stop()
        threading.Thread(target=target, daemon=True).start()

    def detect_ports(self):
        def extrair_portas(data):
            portas = []

            def add_porta(valor):
                if valor and valor not in portas:
                    portas.append(valor)

            # Arduino CLI mais novo costuma retornar um objeto com "detected_ports".
            if isinstance(data, dict):
                if isinstance(data.get("detected_ports"), list):
                    for item in data["detected_ports"]:
                        if not isinstance(item, dict):
                            continue
                        port_info = item.get("port")
                        if isinstance(port_info, dict):
                            add_porta(port_info.get("address"))
                            add_porta(port_info.get("label"))
                        add_porta(item.get("address"))
                else:
                    port_info = data.get("port")
                    if isinstance(port_info, dict):
                        add_porta(port_info.get("address"))
                    add_porta(data.get("address"))

            # Arduino CLI mais antigo pode retornar uma lista diretamente.
            elif isinstance(data, list):
                for item in data:
                    if not isinstance(item, dict):
                        continue
                    port_info = item.get("port")
                    if isinstance(port_info, dict):
                        add_porta(port_info.get("address"))
                        add_porta(port_info.get("label"))
                    add_porta(item.get("address"))

            return portas

        def task():
            self.append_log("Detectando placas/portas...")
            try:
                result = subprocess.run(
                    [self.cli(), "board", "list", "--format", "json"],
                    capture_output=True,
                    text=True,
                    encoding="utf-8",
                    errors="replace"
                )
                if result.returncode != 0:
                    self.append_log(result.stdout + result.stderr)
                    raise RuntimeError("Falha ao listar portas.")

                data = json.loads(result.stdout or "[]")
                ports = extrair_portas(data)

                self.port_combo["values"] = ports
                if ports:
                    self.port_combo.current(0)
                    self.port_entry.delete(0, tk.END)
                    self.port_entry.insert(0, ports[0])
                    self.append_log("Portas encontradas: " + ", ".join(ports))
                else:
                    self.append_log("Nenhuma porta encontrada. Confira cabo USB/dados e driver.")
                    self.append_log("Dica: rode no terminal: arduino-cli board list")
            except Exception as e:
                self.append_log(f"❌ Erro: {e}")
        threading.Thread(target=task, daemon=True).start()

    def install_avr_core(self):
        self.run_command([self.cli(), "core", "update-index"])
        self.run_command([self.cli(), "core", "install", "arduino:avr"])

    def compile_selected(self):
        item = self.get_selected_item()
        if not item:
            raise RuntimeError("Selecione um sketch.")
        folder = self.resolve_sketch_folder(item["sketch"])
        fqbn = self.fqbn_entry.get().strip()
        self.run_command([self.cli(), "compile", "--fqbn", fqbn, str(folder)])

    def compile_upload_selected(self):
        item = self.get_selected_item()
        if not item:
            raise RuntimeError("Selecione um sketch.")
        port = self.port_entry.get().strip()
        if not port:
            raise RuntimeError("Informe a porta, por exemplo COM3, COM4 ou /dev/ttyACM0.")
        folder = self.resolve_sketch_folder(item["sketch"])
        fqbn = self.fqbn_entry.get().strip()
        self.run_command([self.cli(), "compile", "--fqbn", fqbn, str(folder)])
        self.run_command([self.cli(), "upload", "-p", port, "--fqbn", fqbn, str(folder)])

    def add_sketch(self):
        ino = filedialog.askopenfilename(title="Selecione o .ino", filetypes=[("Arduino sketch", "*.ino")])
        if not ino:
            return
        game = simple_input(self, "Nome do jogo", "Digite o nome do jogo:")
        if not game:
            return
        fqbn = self.fqbn_entry.get().strip() or self.config_data.get("default_fqbn", "arduino:avr:leonardo")
        try:
            rel = str(Path(ino).resolve().relative_to(APP_DIR))
        except ValueError:
            rel = str(Path(ino).resolve())
        self.config_data.setdefault("sketches", []).append({"game": game, "sketch": rel, "fqbn": fqbn})
        save_config(self.config_data)
        self.refresh_sketches()
        self.append_log(f"Sketch adicionado: {game}")


def simple_input(parent, title, prompt):
    win = tk.Toplevel(parent)
    win.title(title)
    win.geometry("360x120")
    win.transient(parent)
    win.grab_set()
    ttk.Label(win, text=prompt).pack(padx=12, pady=(12, 6), anchor="w")
    entry = ttk.Entry(win)
    entry.pack(fill="x", padx=12)
    result = {"value": None}
    def ok():
        result["value"] = entry.get().strip()
        win.destroy()
    ttk.Button(win, text="OK", command=ok).pack(pady=10)
    entry.focus()
    parent.wait_window(win)
    return result["value"]


if __name__ == "__main__":
    app = ArduinoUploaderApp()
    app.mainloop()

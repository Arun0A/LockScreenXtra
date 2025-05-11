import tkinter as tk
import random

boot_lines = [
    "[  OK  ] Started Network Manager",
    "[  OK  ] Mounted /boot",
    "[  OK  ] Started GNOME Display Manager",
    "[FAILED] Failed to start Load Kernel Modules",
    "[  OK  ] Started Authorization Manager",
    "[  OK  ] Reached target Graphical Interface",
    "[  OK  ] Started Login Service",
    "[  OK  ] Started User Manager for UID 1000",
    "[  OK  ] Started Session c1 of user archuser"
]

def generate_fake_log():
    return f"[    {round(random.uniform(0.00001, 3.99999), 5)}] {random.choice(['usb', 'kernel', 'systemd', 'udevd'])}: {random.choice(['Device ready', 'Initializing', 'Mounting', 'Daemon started'])}"

class BootSimulator:
    def __init__(self, root):
        self.root = root
        root.title("Secure System Boot")
        root.attributes("-fullscreen", True)
        root.configure(bg="black")
        root.bind("<Escape>", lambda e: self.close_program())

        # Use a Frame to stack widgets vertically
        self.frame = tk.Frame(root, bg="black")
        self.frame.pack(fill="both", expand=True)

        self.text = tk.Text(self.frame, bg="black", fg="white", insertbackground="white",
                            font=("Courier", 14), borderwidth=0)
        self.text.pack(side="top", fill="both", expand=True)
        self.text.config(state=tk.DISABLED)

        self.entry = tk.Entry(self.frame, bg="black", fg="white", insertbackground="white",
                              font=("Courier", 14), borderwidth=0, width=41)
        self.entry.bind("<Return>", self.handle_input)

        self.lines_to_print = boot_lines + [generate_fake_log() for _ in range(40)]
        random.shuffle(self.lines_to_print)
        self.line_index = 0
        self.login_stage = 0

        self.root.after(500, self.print_next_line)

    def print_next_line(self):
        if self.line_index < len(self.lines_to_print):
            self.append_line(self.lines_to_print[self.line_index])
            self.line_index += 1
            self.root.after(80 + random.randint(20, 100), self.print_next_line)
        else:
            self.ask_login()

    def append_line(self, line):
        self.text.config(state=tk.NORMAL)
        self.text.insert(tk.END, line + "\n")
        self.text.see(tk.END)
        self.text.config(state=tk.DISABLED)

    def ask_login(self):
        self.append_line("\nArun's Secure System Login: ")
        self.entry.pack(side="bottom", anchor="w", padx=0)
        self.entry.focus()

    def handle_input(self, event):
        input_text = self.entry.get()
        self.entry.delete(0, tk.END)
        self.entry.pack_forget()

        if self.login_stage == 0:
            self.append_line(input_text)
            self.append_line("Password: ")
            self.login_stage = 1
            self.entry.config(show="*")
            self.entry.pack(side="bottom", anchor="w", padx=0)
            self.entry.focus()
        elif self.login_stage == 1:
            self.append_line("*" * len(input_text))
            self.append_line("\nStarting systemd...\n")
            self.entry.config(state=tk.DISABLED)
            self.entry.pack_forget()
            self.root.after(1000, self.close_program)

    def close_program(self):
        self.root.destroy()

root = tk.Tk()
app = BootSimulator(root)
root.mainloop()

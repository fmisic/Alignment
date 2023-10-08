from tkinter import *
from tkinter import messagebox, filedialog
import subprocess
import queue
import threading
import os
import platform

MSAGA_FILENAME = ""
#MSAGA_FILENAME = "msaga.exe"
MSAGA_FOLDER = ""
#MSAGA_FOLDER = r"...\met\projekt\msaga"

if not MSAGA_FILENAME:
    if platform.system() == "Windows":
        MSAGA_FILENAME = "msaga.exe"
    else:
        MSAGA_FILENAME = "msaga"

if not MSAGA_FOLDER:
    MSAGA_FOLDER = os.path.dirname(__file__)

MSAGA_PATH = os.path.join(MSAGA_FOLDER, MSAGA_FILENAME)

COLOR_TAGS = dict({
    "A": "pale green",
    "R": "red",
    "N": "mediumpurple3",
    "D": "mediumpurple3",
    "C": "khaki",
    "Q": "mediumpurple3",
    "E": "mediumpurple3",
    "G": "orange",
    "H": "violet red",
    "I": "cyan",
    "L": "cyan",
    "K": "red",
    "M": "cyan",
    "F": "green yellow",
    "P": "yellow",
    "S": "green4",
    "T": "green",
    "W": "blue",
    "Y": "green yellow",
    "V": "cyan"
})


class ScrollableFrame(Frame):

    def __init__(self, *args, **kwargs):
        file_dialog = kwargs.pop('file_dialog', False)
        super().__init__(*args, **kwargs)
        self.text = Text(self, bd = 2, wrap = NONE)
        self.vsb = Scrollbar(self, orient = "vertical", command = self.text.yview)
        self.hsb = Scrollbar(self, orient = "horizontal", command = self.text.xview)
        self.text.configure(yscrollcommand = self.vsb.set, xscrollcommand = self.hsb.set)
        self.text.grid(row = 0, column = 0, stick = "NSEW",
                       rowspan = 2, columnspan = 2)
        self.vsb.grid(row = 0, column = 2, rowspan = 2, sticky = "NS")
        self.hsb.grid(row = 2, column = 0, columnspan = 2, sticky = "EW")
        self.file_button = None
        if file_dialog:
            self.file_button = Button(self, text = "Choose file",
                width = 10, bd = 2, bg = "light grey", pady = 2,
                justify = "center", command = self.readFile)
            self.file_button.grid(row = 1, column = 1, stick = "SE")
        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(0, weight=1)
        self.grid_propagate(False)        
        return

    def clear(self):
        self.text.delete("1.0", "end")
        return

    def readFile(self):
        filename = filedialog.askopenfilename(parent = self,
            filetypes = (("Normal text file", ".txt"), ("All files", "*.*")))
        if filename:
            with open(filename) as file:
                text = file.read()
            self.clear()
            self.text.insert("end", text)
        return
        


class AlignmentFrame(ScrollableFrame):

    def __init__(self, *args, **kwargs):
        kwargs.setdefault('file_dialog', True)
        super().__init__(*args, **kwargs)
        self.text.config(font = ("Courier New", 13))
        for color in COLOR_TAGS.values():
            self.text.tag_configure(color, background = color)
        return

    def addText(self, text):
        self.text.insert("end", text)
        return

    def addAlignment(self, lines):
        self.clear()
        self.addText("\n".join(lines))
        return

    def addAlignmentSolution(self, lines, score):
        self.clear()
        score_line = "Score: {}".format(score)
        lines.append("")
        lines.append(score_line)
        self.addText("\n".join(lines))
        self.color()
        return

    def color(self):
        lines = self.text.get("1.0", "end-1c").split("\n")
        for tag in COLOR_TAGS.values():
            self.text.tag_remove(tag, "1.0", "end")
        for i, line in enumerate(lines):
            if not line or line.startswith("Score:"):
                continue
            for j in range(len(line)):
                tag = COLOR_TAGS.get(line[j], None)
                if tag:
                    self.text.tag_add(tag,
                        "{}.{}".format(i+1, j), "{}.{}".format(i+1, j+1))
        return

    def getAlignmentText(self):
        alignment_text = self.text.get("1.0", "end-1c")
        alignment_lines = [line for line in alignment_text.split("\n")
                           if line.strip() and not line.startswith("Score:")]
        alignment_text = "\n".join(alignment_lines)
        alignment_text = alignment_text.replace("-", "")
        return alignment_text
    

class ConsoleFrame(ScrollableFrame):

    def __init__(self, *args, **kwargs):     
        super().__init__(*args, **kwargs)
        self.queue = queue.Queue()
        self.text.config(bg = "gray20", fg = "gray80",
                         font = ("Courier New", 10))
        self.hidden = True
        self.writing = False
        return

    def click(self, button):
        self.button = button
        if self.hidden:
            self.show()            
        else:
            self.hide()
        return
        
    def show(self):
        self.button.config(text = "Hide Console")
        screenwidth = self.master.winfo_screenwidth()
        width = min(1400, screenwidth)
        self.master.geometry("{}x700".format(width))##
        self.master.minsize(width = width, height = 700)##
        self.master.columnconfigure(3, weight = 20)##
        self.grid()
        self.hidden = False
        return

    def hide(self):
        self.button.config(text = "Show Console")
        self.master.geometry("1000x700")
        self.master.minsize(width = 1000, height = 700)
        self.master.columnconfigure(3, weight = 0)
        self.grid_remove()
        self.hidden = True
        return

    def write(self, text):
        self.queue.put(text)
        return

    def startWriting(self):
        self.clear()
        self.writing = True
        self.writingBuffer()
        return

    def finshWriting(self):
        if self.writing:
            self.writing = False
            while True:
                try:
                    text = self.queue.get_nowait()
                    self.print(text)
                except queue.Empty:
                    break
        return

    def writingBuffer(self):
        buffer_time = 500
        while True:
            try:
                text = self.queue.get_nowait()
                self.print(text)
                if "\n" in text:
                    buffer_time = 10
                    break
            except queue.Empty:
                break
        if self.writing:
            self.after(buffer_time, self.writingBuffer)
        return

    def print(self, text):
        for c in text:
            if c != "\x08":
                self.text.insert("end", c)
            else:
                self.text.delete("end-2c", "end-1c")
        self.text.update()
        if not all(c in '#.:-=\x08' for c in text):
            self.text.see("end-1c linestart")
        return


class SolutionWindow(Toplevel):

    def __init__(self, *args, **kwargs):     
        super().__init__(*args, **kwargs)
        self.withdraw()
        self.title("Solution alignment")
        self.frame = AlignmentFrame(self)
        self.save_button = Button(self, text = "Save", width = 15,
                                  bd = 2, bg = "light grey", pady = 2,
                                  justify = "center", command = self.save)
        self.add_button = None
        self.alignment_frame = None
        self.inital_text = None
        self.solution_text = None
        self.frame.grid(column = 0, row = 0, sticky = "NSEW")
        self.save_button.grid(column = 0, row = 1)
        self.columnconfigure(0, weight = 1)
        self.rowconfigure(0, weight = 1)
        self.geometry('800x500')
        self.protocol("WM_DELETE_WINDOW", self.close)
        return

    def convertFromClustalw(self, clustalw_lines):
        alignment_lines = []
        for line in clustalw_lines:
            if line.startswith(">"):
                alignment_lines.append("\n")
            else:
                alignment_lines.append(line.strip())
        alignment_text = "".join(alignment_lines).strip()
        return alignment_text

    def save(self):
        alignment_text = self.alignment_frame.getAlignmentText()
        solution_text = self.frame.text.get("1.0", "end-1c").strip()
        lines = solution_text.split("\n")
        if any(line.startswith('>') for line in lines):
               solution_text = self.convertFromClustalw(lines)
        solution_alignment_text = solution_text.replace("-", "")
        save = True
        if alignment_text:
            solution_alignment_lines = sorted(solution_alignment_text.split("\n"))
            alignment_lines = sorted(alignment_text.split("\n"))
            if solution_alignment_lines != alignment_lines:
                save = messagebox.askyesno("Wrong solution alignment",
                    "Solution alignment and input alignment do not match.\n" + \
                    "Do you want to use it anyway?", parent = self)
        if save:
            if solution_alignment_text:
                self.alignment_frame.clear()
                self.alignment_frame.addText(solution_alignment_text)
                self.solution_text = solution_text
                self.add_button.config(text = "Change Solution")
            else:
                self.solution_text = None
                self.add_button.config(text = "Add Solution")
            self.withdraw()
        return

    def add(self):
        self.inital_text = self.frame.text.get("1.0", "end-1c")
        self.deiconify()
        return

    def connectAddButton(self, add_button):
        self.add_button = add_button
        return

    def connectAlignmentFrame(self, alignment_frame):
        self.alignment_frame = alignment_frame
        return

    def close(self):
        self.withdraw()
        self.frame.clear()
        if self.inital_text:
            self.frame.text.insert("end", self.inital_text)
            self.inital_text = None
        return


class Msaga:

    def __init__(self, path):
        self.path = path
        self.folder = os.path.dirname(path)
        self.text_list = []
        self.alignment_frame = None
        self.console_frame = None
        self.button = None
        self.gennum_entry = None
        self.popnum_entry = None
        self.popsize_entry = None
        self.gapcoeff_entry = None
        self.mutation_entry = None
        self.mutation1_entry = None
        self.mutation2_entry = None
        self.mutation3_entry = None
        self.score_var = None
        self.fitness_var = None
        self.add_button = None
        self.solution_window = None
        self.result = []
        self.running = False
        self.finsh_writing_id = None
        self.input_file_path = os.path.join(self.folder, "input_file.txt")
        self.solution_file_path = os.path.join(self.folder, "solution_file.txt")
        return

    def connectFrames(self, alignment_frame, console_frame):
        self.alignment_frame = alignment_frame
        self.console_frame = console_frame
        return

    def connectEntries(self, gennum_entry, popnum_entry, popsize_entry, gapcoeff_entry,
                   mutation_entry, mutation1_entry, mutation2_entry, mutation3_entry,
                   score_var, fitness_var):
        self.gennum_entry = gennum_entry
        self.popnum_entry = popnum_entry
        self.popsize_entry = popsize_entry
        self.gapcoeff_entry = gapcoeff_entry
        self.mutation_entry = mutation_entry
        self.mutation1_entry = mutation1_entry
        self.mutation2_entry = mutation2_entry
        self.mutation3_entry = mutation3_entry
        self.score_var = score_var
        self.fitness_var = fitness_var
        return

    def connectAddButton(self, add_button):
        self.add_button = add_button
        return

    def connectSolutionWindow(self, solution_window):
        self.solution_window = solution_window
        return
    
    def click(self, button):
        self.button = button
        if self.running:
            self.stop()
        else:
            self.run()
        return

    def run(self):
        self.button.config(text = "STOP")
        self.console_frame.finshWriting()
        if self.finsh_writing_id:
            app = self.console_frame.master
            app.after_cancel(self.finsh_writing_id)
        args = [self.path]
        alignment_text = self.alignment_frame.getAlignmentText()
        solution = False
        if self.add_button["text"] == "Change Solution":
            solution_text = self.solution_window.solution_text
            solution_alignment_text = solution_text.replace("-", "")
            if solution_alignment_text == alignment_text:
                solution = True
            else:
                self.add_button.config(text = "Add Solution")
                self.solution_window.frame.clear()
        if solution:
            with open(self.solution_file_path, "w") as file:
                file.write(solution_text)
            args.append(self.solution_file_path)
            args.append("-solution")
        else:
            with open(self.input_file_path, "w") as file:
                file.write(alignment_text)
            args.append(self.input_file_path)
        if self.gennum_entry.get():
            args.extend(["-gennum", self.gennum_entry.get()])
        if self.popnum_entry.get():
            args.extend(["-popnum", self.popnum_entry.get()])
        if self.popsize_entry.get():
            args.extend(["-popsize", self.popsize_entry.get()])
        if self.gapcoeff_entry.get() and not solution:
            args.extend(["-gapcoeff", self.gapcoeff_entry.get()])
        if any(me.get() for me in [self.mutation_entry, self.mutation1_entry,
                             self.mutation2_entry, self.mutation2_entry]):
            mp = self.mutation_entry.get() or "0.1"
            mp1 = self.mutation1_entry.get() or "0.3"
            mp2 = self.mutation2_entry.get() or "0.3"
            mp3 = self.mutation3_entry.get() or "0.3"
            args.extend(["-mparams", mp, mp1, mp2, mp3])
        args.extend(["-fitness", str(self.score_var.get())])
        if self.fitness_var.get() == 1:
            args.extend(["-score", "blosum62"])
        if self.fitness_var.get() == 2:
            args.extend(["-score", "pam250"])
        print_args = " ".join([os.path.basename(args[0]),
            os.path.basename(args[1])] + args[2:]) + "\n\n"
        self.console_frame.write(print_args)
        self.running = True
        flags = 0
        if platform.system() == "Windows":
            flags = subprocess.CREATE_NO_WINDOW
        self.process = subprocess.Popen(args, cwd = self.folder,
            stdout = subprocess.PIPE, stderr = subprocess.STDOUT,
            creationflags = flags)
        task = threading.Thread(target = self.reader)
        task.start()
        self.console_frame.startWriting()
        self.checkIfFinished()
        return

    def reader(self):
        self.text_list.clear()
        try:
            for text in iter(lambda: self.process.stdout.read(1), b""):
                if self.running:
                    self.processText(text)
                else:
                    subprocess.run("TASKKILL /F /PID {pid} /T".
                                   format(pid = self.process.pid))
                    break
        except Exception as e:
            print(e)
        return

    def processText(self, text):
        text = text.decode("utf-8")
        self.text_list.append(text)
        self.console_frame.write(text)
        return

    def stop(self):
        self.stopRunning()
        text = "".join(self.text_list).replace("\r", "")
        lines = text.split("\n")
        i = len(lines) - 1
        while i > 0:
            if set(lines[i]) == {"-"}:
                break
            i -= 1
        self.text_list = ["\n".join(lines[:i])]
        self.afterFinished()
        return

    def stopRunning(self):
        app = self.console_frame.master
        self.finsh_writing_id = app.after(1000, self.console_frame.finshWriting)
        self.button.config(text = "RUN")
        self.running = False
        return
        

    def checkIfFinished(self):
        if self.process.poll() == 0:
            self.afterFinished()
            self.stopRunning()
        if self.running:
            app = self.console_frame.master
            app.after(500, self.checkIfFinished)
        return

    def afterFinished(self):
        text = "".join(self.text_list).replace("\r", "")
        lines = text.split("\n")
        self.result.clear()
        generation = None
        key = None
        i = 0
        while i < len(lines):
            line = lines[i]
            if not line:
                pass
            elif line.startswith("Population"):
                if generation:
                    self.result.append(generation)
                generation = dict()
            elif line.startswith("Score range of initialized alignments"):
                score_range = [int(n.strip()) for n in line.
                               split(":")[1].strip().strip("[]").split(",")]
                generation["initial"] = dict({"range": score_range})
                key = "initial"
            elif line.startswith("Score range of alignments in last generation"):
                score_range = [int(n.strip()) for n in line.
                               split(":")[1].strip().strip("[]").split(",")]
                generation["final"] = dict({"range": score_range})
                key = "final"
            elif line.startswith("Alignment with highest score"):
                alignment_lines = []
                i += 1
                while not lines[i].startswith("Score"):
                    alignment_lines.append(lines[i])
                    i += 1
                score = int(lines[i].split(":")[1].strip())
                generation[key]["alignment"] = alignment_lines
                generation[key]["score"] = score
            i += 1
        if generation:
            self.result.append(generation)
        self.showBestSolution()
        return

    def showBestSolution(self):
        if self.result:
            best = max(self.result, key = lambda x: x["final"]["score"])
            self.alignment_frame.addAlignmentSolution(
                best["final"]["alignment"], best["final"]["score"])
        return

    


msaga = Msaga(MSAGA_PATH)
    
root = Tk()
root.title("Multiple sequence alignment by genetic algorithm")
root.geometry("1000x700")
root.minsize(width = 1000, height = 700)
root.resizable(width = True, height = False)
root.grid_columnconfigure(0, weight = 1)


# Solution window
solution_window = SolutionWindow(root)

msaga.connectSolutionWindow(solution_window)


#frames
frame1_1 = Frame(root, height = 40, width = 260)
frame1_2 = Frame(root, height = 40, width = 260)
frame1_3 = Frame(root, height = 40, width = 260)
frame1_4 = Frame(root, height = 40, width = 260)
frame2_1 = Frame(root, height = 40, width = 260)
frame2_2 = Frame(root, height = 40, width = 260)
frame2_3 = Frame(root, height = 40, width = 260)
frame2_4 = Frame(root, height = 40, width = 260)
frame3_1 = Frame(root, height = 80, width = 260)
frame3_2 = Frame(root, height = 80, width = 260)
frame4 = AlignmentFrame(root, height = 400, width = 800)
frame5_1  = Frame(root, height = 80, width = 220)
frame5_2 = Frame(root, height = 80, width = 220)
frame5_3 = Frame(root, height = 80, width = 220)
frame6 = ConsoleFrame(root, height = 600)

msaga.connectFrames(frame4, frame6)


#parameters 1
label1_1 = Label(frame1_1, text = "Number of generations:")
entry1_1 = Entry(frame1_1, width = 10, justify = "right")
entry1_1.insert(0, "100")

label1_2 = Label(frame1_2, text = "Number of populations:")
entry1_2 = Entry(frame1_2, width = 10, justify = "right")
entry1_2.insert(0, "10")

label1_3 = Label(frame1_3, text = "Population size:")
entry1_3 = Entry(frame1_3, width = 10, justify = "right")
entry1_3.insert(0, "100")

label1_4 = Label(frame1_4, text = "Gap coefficient:")
entry1_4 = Entry(frame1_4, width = 10, justify = "right")
entry1_4.insert(0, "1.2")


#parameters 2
label2_1 = Label(frame2_1, text = "Mutation probability:")
entry2_1 = Entry(frame2_1, width = 10, justify = "right")
entry2_1.insert(0, "0.1")

label2_2 = Label(frame2_2, text = "Mutation #1 probability:")
entry2_2 = Entry(frame2_2, width=10, justify = "right")
entry2_2.insert(0, "0.3")

label2_3 = Label(frame2_3, text = "Mutation #2 probability:")
entry2_3 = Entry(frame2_3, width = 10, justify = "right")
entry2_3.insert(0, "0.3")

label2_4 = Label(frame2_4, text = "Mutation #3 probability:")
entry2_4 = Entry(frame2_4, width = 10, justify = "right")
entry2_4.insert(0, "0.3")


#fitness function
f = IntVar()
f.set(1)
label3_1 = Label(frame3_1, text = "Choose fitness function:")
radiobutton3_1 = Radiobutton(frame3_1, text = "Option 1", variable = f, value = 1)
radiobutton3_2 = Radiobutton(frame3_1, text = "Option 2", variable = f, value = 2)


#Scoring matrix
m = IntVar()
m.set(1)
label3_2 = Label(frame3_2, text = "Choose scoring matrix:")
radiobutton3_3=Radiobutton(frame3_2, text = "BLOSUM 62", variable = m, value = 1)
radiobutton3_4=Radiobutton(frame3_2, text = "PAM 250", variable = m, value = 2)


msaga.connectEntries(entry1_1, entry1_2, entry1_3, entry1_4,
                 entry2_1, entry2_2, entry2_3, entry2_4, f, m)

button5_1 = Button(frame5_1, text = "Add Solution", width = 15, bd = 2,
                   bg = "light grey", pady = 2, justify = "center")
button5_1.config(command = solution_window.add)

button5_2 = Button(frame5_2, text = "RUN", width = 20, bg = "green", bd = 5,
                   pady = 5, justify = "center", fg = "white")
button5_2.config(command = lambda: msaga.click(button5_2))

button5_3 = Button(frame5_3, text = "Show Console", width = 15, bd = 2,
                   bg = "light grey", pady = 2, justify = "center")
button5_3.config(command = lambda: frame6.click(button5_3))

solution_window.connectAlignmentFrame(frame4)
solution_window.connectAddButton(button5_1)
msaga.connectAddButton(button5_1)


root.columnconfigure(0, weight = 1)
root.columnconfigure(1, weight = 1)
root.columnconfigure(2, weight = 1)
root.rowconfigure(0, weight = 1)
root.rowconfigure(5, weight = 1)
root.rowconfigure(6, weight = 1)
root.rowconfigure(7, weight = 1)

frame1_1.grid(row = 1, column = 0)
frame1_2.grid(row = 2, column = 0)
frame1_3.grid(row = 3, column = 0)
frame1_4.grid(row = 4, column = 0)
frame2_1.grid(row = 1, column = 1)
frame2_2.grid(row = 2, column = 1)
frame2_3.grid(row = 3, column = 1)
frame2_4.grid(row = 4, column = 1)
frame3_1.grid(row = 1, column = 2, rowspan = 2)
frame3_2.grid(row = 3, column = 2, rowspan = 2)
frame4.grid(row = 5, column = 0, columnspan = 3)
frame5_1.grid(row = 6, column = 0)
frame5_2.grid(row = 6, column = 1)
frame5_3.grid(row = 6, column = 2)
frame6.grid(row = 1, column = 3, rowspan = 6, sticky = "NSEW")
frame6.grid_remove()


for frame in [frame1_1, frame1_2, frame1_3, frame1_4,
              frame2_1, frame2_2, frame2_3, frame2_4]:
    frame.columnconfigure(0, minsize = 170)
    frame.columnconfigure(1, minsize = 90)
    
for frame in [frame3_1, frame3_2]:
    for i in [0, 1]:
        frame.columnconfigure(i, minsize = 130)
        frame.rowconfigure(i, minsize = 40)

frame5_3.columnconfigure(0, minsize = 220)


label1_1.grid(row = 0, column = 0, sticky = W)
entry1_1.grid(row = 0, column = 1, sticky = E)
label1_2.grid(row = 0, column = 0, sticky = W)
entry1_2.grid(row = 0, column = 1, sticky = E)
label1_3.grid(row = 0, column = 0, sticky = W)
entry1_3.grid(row = 0, column = 1, sticky = E)
label1_4.grid(row = 0, column = 0, sticky = W)
entry1_4.grid(row = 0, column = 1, sticky = E)

label2_1.grid(row = 0, column = 0, sticky = W)
entry2_1.grid(row = 0, column = 1, sticky = E)
label2_2.grid(row = 0, column = 0, sticky = W)
entry2_2.grid(row = 0, column = 1, sticky = E)
label2_3.grid(row = 0, column = 0, sticky = W)
entry2_3.grid(row = 0, column = 1, sticky = E)
label2_4.grid(row = 0, column = 0, sticky = W)
entry2_4.grid(row = 0, column = 1, sticky = E)

label3_1.grid(row = 0, column = 0, columnspan = 2, sticky = W)
radiobutton3_1.grid(row = 1, column = 0)
radiobutton3_2.grid(row = 1, column = 1)

label3_2.grid(row = 0, column = 0, columnspan = 2, sticky = W)
radiobutton3_3.grid(row = 1, column = 0)
radiobutton3_4.grid(row = 1, column = 1)

button5_1.grid(row = 0, column = 0)
button5_2.grid(row = 0, column = 0)
button5_3.grid(row = 0, column = 0, sticky = E)



root.mainloop()

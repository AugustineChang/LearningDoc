import sys
import tkinter
import tkinter.messagebox

def main():
    root = tkinter.Tk()
    
    root.title("First Window!!")
    root.geometry("500x300")
    root.resizable(False,False)
    
    
    bar = tkinter.Menu(root)
    
    #File
    def quit():
        root.destroy()    
    
    fileMenu = tkinter.Menu(bar, tearoff=0)
    fileMenu.add_command(label="Exit", command=quit)
    bar.add_cascade(label="File", menu=fileMenu)
    
    #Help
    def aboutUs():
        tkinter.messagebox.showinfo("About", "Made by zhanghu!")
        
    helpMenu = tkinter.Menu(bar, tearoff=0)
    helpMenu.add_command(label="About", command=aboutUs)
    bar.add_cascade(label="Help", menu=helpMenu)
    
    root.config(menu=bar)
    
    #Content
    mainFrame = tkinter.Frame(root, borderwidth=1, padx=5, pady=5)
    mainFrame.pack()
    
    note = tkinter.Text(mainFrame, bg="white", height=20)
    note.pack(fill="x")
    
    def ClickBtn():
        note.insert("end", "Button is Clicked!!\n")
    
    btn = tkinter.Button(mainFrame, text="Click Me!",command=ClickBtn)
    btn.pack()
    
    root.mainloop()

if __name__ == "__main__":
    main()
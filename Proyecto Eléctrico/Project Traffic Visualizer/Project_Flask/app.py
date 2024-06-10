from flask import Flask, render_template, request, redirect

import os

from werkzeug.utils import secure_filename

app = Flask(__name__)
app.debug = True

@app.route('/')
def test():
    return render_template('test.html')


app.config["UPLOADS"] = "./uploads"

def confirm_json(filename):

    if not "." in filename:
        return False

    ext = filename.rsplit(".", 1)[1]

    print(ext.upper())
    if ext.upper() == "JSON":
        return True
    else:
        return False

@app.route('/insert', methods=["GET","POST"])
def insert():

    if request.method == "POST":

        if request.files:

            jsonfile = request.files["jsonfile"]

            print(jsonfile.filename)

            if jsonfile.filename == "":
                print("File must have a filename")
                return redirect(request.url)

            if not confirm_json(jsonfile.filename):
                print("Not a json file uploaded")
                return redirect(request.url)

            else:
                jsonfilename = secure_filename(jsonfile.filename)
                jsonfile.save(os.path.join(app.config["UPLOADS"], jsonfilename))
                
            print("Saved")
            return redirect(request.url)

    return render_template('insert.html')



if __name__ == '__main__':
    app.run()
from flask import Flask, render_template

app = Flask(__name__,template_folder='templates')


@app.route('/')
@app.route('/hello.html')
def hello_world():
    """Hello world function

    Returns template:
        hello.html
    """
    return render_template('hello.html')
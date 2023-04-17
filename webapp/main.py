from flask import Flask, request, render_template, redirect, flash
from flask import make_response, url_for, jsonify
from flask_bootstrap import Bootstrap
import requests
import json


app = Flask(__name__)
bootstrap = Bootstrap(app)

app.secret_key = "0000"
IP = 'http://192.168.1.204/drink'

# Premade cocktail ingredients
mojitoJSON = { "1":0, "2":0, "3":150, "4":100 }
pinaColadaJSON = { "1":50, "2":50, "3":100, "4":50 }
longIslandIceTeaJSON = { "1":100, "2":50, "3":50, "4":50 }
sexOnTheBeachJSON = { "1":100, "2":50, "3":100, "4":0 }

# Homepage
@app.route('/')
def index():
    return render_template('index.html')

# Create your personalised drink page
@app.route('/manually', methods=['GET', 'POST'])
def manually():
    if request.method == 'POST':
        name = request.form.get("name")
        drink1 = request.form.get("rd")
        if drink1 == "":                                                                   # Set value to 0 if user didn't fill in an amount
            drink1 = "0"
        drink2 = request.form.get("bl")
        if drink2 == "":
            drink2 = "0"
        drink3 = request.form.get("lg")
        if drink3 == "":
            drink3 = "0"
        drink4 = request.form.get("dm")
        if drink4 == "":
            drink4 = "0"
        personalizedJSON = { "name":name, "1":drink1, "2":drink2, "3":drink3, "4":drink4 } # Complete personalized drink
        if request.form.get('make') == 'Make':                                             # check which button is pressed
            if drink1.isdigit() and drink2.isdigit() and drink3.isdigit() and drink4.isdigit() and name != "":
                enteredAmount = int(drink1)+int(drink2)+int(drink3)+int(drink4)
                if enteredAmount <= 250:                                                      # check if drink is valid
                    flash('Making your personalized cocktail...')                              # display message
                    try:                                                                       # use try catch block to avoid errors
                        requests.post(IP, json = personalizedJSON)                             # HTTP POST to send json to ESP
                    except requests.exceptions.RequestException as err:
                        flash('An error occurred while making your cocktail')
                else:
                    flash('WARNING: DRINK IS TOO LARGE')                         # display error message to user
                    return redirect(url_for('manually'))
            else:
                flash('WARNING: INVALID INPUT')
        if request.form.get('save') == 'Save':                                             # check which button is pressed
            if drink1.isdigit() and drink2.isdigit() and drink3.isdigit() and drink4.isdigit() and name != "":
                enteredAmount = int(drink1)+int(drink2)+int(drink3)+int(drink4)
                if(enteredAmount <= 250):                                                      # check if drink is valid
                    with open('database.json') as json_file:                                   # open database.json file
                        data = json.load(json_file)                                            # load content into dictionary
                    key = 'drink_' + str(len(data)+1)                                          # create new key
                    data[key] = personalizedJSON                                               # add drink to dictionary
                    with open('database.json', 'w') as fp:                                     # open json file
                        json.dump(data, fp)                                                    # replace content with content of the dictionary 
                    flash('Drink called: ' + name + ' is saved successfully')                  # display successful message
                else:
                    flash('WARNING: DRINK IS TOO LARGE')                                              # display error message
            else:
                flash('WARNING: INVALID INPUT')
    return render_template('manually.html')

# Saved drinks page
@app.route('/savedDrinks', methods=['GET', 'POST'])
def savedDrinks():
    with open('database.json') as json_file:                                                            # open json file and load content into dictionary
        data = json.load(json_file)
    size = len(data)
    if request.method == 'POST':
        for i in data:                                                                                  # iterate over all elements in the dictionary
            if request.form.get(data[i]['name']) == 'Make':                                             # check to see which make button is pressed and make the right drink
                try:
                    makeDrink={"1":data[i]['1'], "2":data[i]['2'], "3":data[i]['3'], "4":data[i]['4']}
                    requests.post(IP, json=makeDrink)
                except requests.exceptions.RequestException as err:
                    print ("Error making " + data[i]['name'])
            else:
                pass
            if request.form.get(data[i]['name']) == 'Delete':                                           # check to see which delete button is pressed
                for idx, obj in enumerate(data):                                                        # enumerate over the dictionary to find the right element
                    if data[obj]['name'] == data[i]['name']:
                        data.pop(obj)                                                                   # pop the element from the dictionary
                        number = 1
                        newData = {}
                        for j in data:                                                                  # restore the names of the dictionary (drink_1, drink_3 turns into drink_1, drink_2)
                            key = "drink_"+str(number)
                            newData[key] = data[j]
                            number = number+1
                        with open('database.json', 'w') as fp:                                          # overwrite the database with the updated dictionary
                            json.dump(newData, fp)
                        return render_template('savedDrinks.html', data=newData, size=size)             # imediately render the template again to see the changes
            else:
                pass
    return render_template('savedDrinks.html', data=data, size=size)

# List of premade cocktails page
@app.route('/cocktail', methods=['GET', 'POST'])
def cocktail():
    return render_template('cocktail.html')

# Renders Mojito page and sends HTTP POST when 'make' button is pressed
@app.route('/cocktail1', methods=['GET', 'POST'])
def cocktail1():
    if request.method == 'POST':
        if request.form.get('sendPOST1') == 'Make Mojito!':
            print("Make Mojito")
            try:
                requests.post(IP, json = mojitoJSON)
            except requests.exceptions.RequestException as err:
                print ("Error making mojito")
        else:
            pass
    return render_template('cocktail1.html')

# Renders Pina Colada page and sends HTTP POST when 'make' button is pressed
@app.route('/cocktail2', methods=['GET', 'POST'])
def cocktail2():
    if request.method == 'POST':
        if request.form.get('sendPOST2') == 'Make Pina Colada!':
            print("Make Pina Colada")
            try:
                requests.post(IP, json = pinaColadaJSON)
            except requests.exceptions.RequestException as err:
                print ("Error making pina coladat")
        else:
            pass
    return render_template('cocktail2.html')

# Renders Long Island Iced Tea page and sends HTTP POST when 'make' button is pressed
@app.route('/cocktail3', methods=['GET', 'POST'])
def cocktail3():
    if request.method == 'POST':
        if request.form.get('sendPOST3') == 'Make Long Island Ice Tea!':
            print("Make Long Island Ice Tea")
            try:
                requests.post(IP, json = longIslandIceTeaJSON)
            except requests.exceptions.RequestException as err:
                print ("Error making long island ice tea")
        else:
            pass
    return render_template('cocktail3.html')

# Renders sex on the beach page and sends HTTP POST when 'make' button is pressed
@app.route('/cocktail4', methods=['GET', 'POST'])
def cocktail4():
    if request.method == 'POST':
        if request.form.get('sendPOST4') == 'Make Sex On The Beach!':
            print("Make Sex On The Beach")
            try:
                requests.post(IP, json = sexOnTheBeachJSON)
            except requests.exceptions.RequestException as err:
                print ("Error making sex on the beach")
        else:
            pass
    return render_template('cocktail4.html')

# Handles 404 error
@app.errorhandler(404)
def page_not_found(e):
    return render_template('404.html'), 404

# Handles 500 error
@app.errorhandler(500)
def internal_server_error(e):
    return render_template('500.html'), 500

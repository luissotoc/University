import glob
import re
import json

from django.shortcuts import render, redirect, get_object_or_404
from django.views.generic import ListView

from .forms import upload_form
from .models import uploaded_file, module, packet_record, result_output


# ----- HOME VIEW -----
#Returns the main page
def home(request):
    #Set with all uploaded_file objects
    uploaded_files = uploaded_file.objects.all()

    return render(request, 'TRFC_VSLZR/home.html', {
        'uploaded_files': uploaded_files
    })

# ----- TEST VIEW / DEBUG ONLY -----
#Returns a test page
def test(request):
    #Set with all uploaded_file objects
    uploaded_files = uploaded_file.objects.all()

    return render(request, 'TRFC_VSLZR/test.html', {
        'uploaded_files': uploaded_files
    })


# ----- UPLOAD VIEW -----
#Presents the form to upload files, and once uploaded
#extracts data from that file
def upload(request):
    #Set with all uploaded_file objects
    uploaded_files = uploaded_file.objects.all()

    if request.method == 'POST':
        form = upload_form(request.POST, request.FILES)

        #If data is valid, extract the data from the file
        if form.is_valid():

            form.save()

            #Aux vars
            module_num = 0
            repeated_packet = False
            content = ""
            no_id_num = 0
            drop = False

            #Directory of uploaded files
            root = 'TRFC_VSLZR/uploads/'

            #Load file name
            file_name = request.FILES['json'].name

            #Complete file path
            file_path = root + file_name

            #Open uploaded file
            with open(file_path) as current_file:

                #Deserializes the data from the json document
                file_data = json.loads(current_file.read())
            
                #Loop for each module (if it isnt sdk_version or node_id)
                for module_entry in file_data:
                    if(module_entry != 'sdk_version' and module_entry != 'node_id'):

                        #Loads the info of the module into a variable
                        info = file_data[module_entry]

                        #Creates and saves module objetct
                        mod = module(module_name = module_entry, total_records=info['num_records'], parent_file = uploaded_file.objects.get(json = file_name), module_num = module_num)
                        module_num += 1
                        mod.save()

                        #Loads all the packets related to this file
                        file_packets = packet_record.objects.filter(parent_file__json = file_name)

                        #Load the data of each packet in a variable
                        data = info['record_list']

                        for packet in data:
                            
                            #Gets the current packet id
                            try:
                                current_id = packet['tuser_pkt_id']
                            #If there's no match, returns no id
                            except:
                                current_id = "NO_ID_NUM:" + str(no_id_num)
                                no_id_num += 1

                            #Creates a packet object
                            pkt = packet_record(id_num = current_id, parent_file = uploaded_file.objects.get(json = file_name))
                            

                            #Creates a results object
                            rslt = result_output(parent_module = mod, parent_file = uploaded_file.objects.get(json = file_name), results=content)

                            #Loops over each item of the data
                            for item in packet:

                                #Saves the data of the variables 
                                content = content + item + ": " + packet[item] + "\n"

                                #Search for a drop variable, and if is found and true
                                #saves it in a variable
                                drop_re = re.search(r'drop', item.lower())
                                if drop_re:
                                    if packet[item] == "1":
                                        drop = True

                            #Saves the result of the search in the result object
                            rslt.drop = drop
                            
                            #Checks if the packet already exists in the database
                            for pack in file_packets:
                                if pkt.id_num == pack.id_num:
                                    repeated_packet = True

                            #If it doesnt exists, saves it
                            #if it already exists, loads it
                            if repeated_packet == False:
                                pkt.save()
                            else:
                                pkt = packet_record.objects.get(parent_file__json = file_name, id_num = current_id)

                            #Relates the current module with the packet
                            pkt.modules.add(mod)

                            #Complete and save the results object data
                            rslt.parent_packet = pkt
                            rslt.results = content
                            rslt.save()
                            #Reset variables
                            repeated_packet = False
                            content = ""
                            drop = False

            #At the end, redirects to File list page
            return redirect('app-avail')

    else:
        #If method isnt POST, just fills an empty form
        form = upload_form()

    return render(request, 'TRFC_VSLZR/upload.html', {
        'uploaded_files': uploaded_files,
        'form': form
        })

# ----- FILE_LIST VIEW -----
#Returns a list of all files uploaded, with links to each files page
class file_list(ListView):
    model = uploaded_file
    template_name = 'TRFC_VSLZR/available.html'
    context_object_name = 'uploaded_files'
    ordering = ['-date']


# ----- FILE_PAGE VIEW -----
#Presents the selection of packets and modules, for the user to track
#and choose, and opens a new tab with the info selected

def file_page(request, id):
    #Set with all uploaded_file objects
    uploaded_files = uploaded_file.objects.all()


    # fetch the object related to passed id 
    up_file = get_object_or_404(uploaded_file, id = id) 

    #Gets current file name
    file_name = up_file.json

    #Gets list modules and packets in current file
    modules_on_file = module.objects.filter(parent_file__json=file_name)
    packets_on_file = packet_record.objects.filter(parent_file__json=file_name)

    #List of modules related to the selected packet
    modules_tracked = []

    #Selection variables
    s_packet = "null"
    chck_drop = False

    #List of result objects related to modules and packet, for context
    rslt= []

    #initializates the rslt list
    for mod in modules_on_file:
        rslt.append("")

    if request.method == "POST":
        #Gets user selection
        p_proxy = request.POST.get('selected_packet')
        s_drop = request.POST.get('show_dropped')

        #Makes check box into bool
        if s_drop == "on":
            chck_drop = True
        else:
            chck_drop = False 

        #If a packet is selected, makes a list with the modules related to that packet
        if (p_proxy != None): 
            if (p_proxy != "null"):
                s_packet = request.POST.get('selected_packet')
                pkt = packet_record.objects.get(parent_file__json = file_name, id_num = s_packet)
                modules_tracked = pkt.modules.all()

        #Then, load the results object related to the selected packet
        #and each module in the file
        for i, mod in enumerate(modules_on_file):
            try:
                rslt[i] = result_output.objects.get(parent_file__json=file_name, parent_packet__id_num = s_packet, parent_module__module_name = mod.module_name)
            except result_output.DoesNotExist:
                rslt[i] = ""

    else:        
        #If method isnt POST, just resets the variables
        s_packet = "null"
        chck_drop = False
        

    return render(request, 'TRFC_VSLZR/file_page.html', {
        'uploaded_files': uploaded_files,
        'current_file': up_file,
        'modules_on_file': modules_on_file,
        'modules_tracked': modules_tracked,        
        'packets_on_file': packets_on_file,
        'results': rslt,
        's_packet': s_packet,
        'chck_drop': chck_drop
    })


# ----- MODULE_FILE_PAGE VIEW -----
#Presents the selection of packets and modules, for the user to track
#and choose, and opens a new tab with the info selected

def module_file_page(request, id):

    #Set with all uploaded_file objects
    uploaded_files = uploaded_file.objects.all()


    # fetch the object related to passed id 
    up_file = get_object_or_404(uploaded_file, id = id) 

    #Gets current file name
    file_name = up_file.json

    #Gets list modules and packets in current file
    modules_on_file = module.objects.filter(parent_file__json=file_name)\

    #List of modules related to the selected packet
    packets_tracked = []

    #Selection variables
    s_module = "null"
    chck_drop = False

    #Search item
    search_item = ""
    packets_to_show = packet_record.objects.filter(parent_file__json=file_name)

    #List of result objects related to modules and packet, for context
    rslt=[]

    #initializates the rslt list
    for packet in packets_to_show:
        rslt.append("")

    if request.method == "POST":
        #Gets user selection
        m_proxy = request.POST.get('selected_module')
        s_drop = request.POST.get('show_dropped')

        #Get user search
        search_item = request.POST.get('search')

        if(search_item != "" or search_item != ""):
            packets_to_show = packet_record.objects.filter(parent_file__json=file_name, id_num=search_item)
        else:
            packets_to_show = packet_record.objects.filter(parent_file__json=file_name)

        #Makes check box into bool
        if s_drop == "on":
            chck_drop = True
        else:
            chck_drop = False 

        #If a packet is selected, makes a list with the modules related to that packet
        if (m_proxy != None): 
            if (m_proxy != "null"):
                s_module = request.POST.get('selected_module')
                packets_tracked = packet_record.objects.filter(parent_file__json = file_name, modules__module_name = s_module)

        #Then, load the results object related to the selected module
        #and each packet in the file
        for i, pkt in enumerate(packets_to_show):
            try:
                rslt[i] = result_output.objects.get(parent_file__json=file_name, parent_packet__id_num = pkt.id_num, parent_module__module_name = s_module)
            except result_output.DoesNotExist:
                rslt[i] = ""


    else:        
        #If method isnt POST, just resets the variables
        s_module = "null"
        chck_drop = False
        

    return render(request, 'TRFC_VSLZR/module_page.html', {
        'uploaded_files': uploaded_files,
        'current_file': up_file,
        'modules_on_file': modules_on_file,
        'packets_tracked': packets_tracked,
        'packets_to_show': packets_to_show,
        'results': rslt,
        's_module': s_module,
        'chck_drop': chck_drop
    })



# ----- DELETE VIEW -----
# Allows to delete an uploaded file object, and all other objects related to it.
def delete_file(request, id): 
    #Set with all uploaded_file objects
    uploaded_files = uploaded_file.objects.all()

    # fetch the object related to passed id 
    up_file = get_object_or_404(uploaded_file, id = id) 

    if request.method == "POST": 
        
        delete_result = "False"

        delete_result = request.POST.get('delete')

        if (delete_result=="True"):
            up_file.delete() 

        return redirect("app-avail") 
  
    return render(request, "TRFC_VSLZR/delete_view.html", {
        'uploaded_file': up_file
    })


def available(request):
    #Set with all uploaded_file objects
    uploaded_files = uploaded_file.objects.all()


    return render(request, 'TRFC_VSLZR/available.html', {
        'uploaded_files': uploaded_files
    })

# ----- RESULTS VIEW -----
# Shows a table with the information of the selected packet.
def results(request, id_file, module_name, id_num):
    #Set with all uploaded_file objects
    uploaded_files = uploaded_file.objects.all()

    # fetch the object related to passed id 
    up_file = get_object_or_404(uploaded_file, id = id_file) 
    result = result_output.objects.get(parent_module__module_name=module_name, parent_packet__id_num=id_num, parent_file__id=id_file)
    mod = module.objects.get(module_name = module_name, parent_file__id = id_file)
    packet = packet_record.objects.get(id_num=id_num, parent_file__id = id_file)

    #Aux variables
    content = ""
    search_item = ""


    #Post in case of a search
    if request.method == "POST":

        #Get the data from the results record object
        data = result.results

        #get the expression to search
        search_item = request.POST.get('search')

        #Divide the result into lines
        div_results = re.split(r'\n', data)
        for line in div_results:
 
            div_line = re.split(":", line)

            #If the line meets the search, it included in the context for the page
            data_re = re.search(search_item.lower(), div_line[0].lower())
            if data_re:
                
                content = content+line+"\n"

    else: 
        # If the request wasnt a post, it selects all the data from the results
        content = result.results


    #The content of the results table is split in two lists
    #One for the variables and one for the values
    content_table = re.split(":|\n", content)
    if content_table[0]=="":
        del content_table[0]
    
    content_variable = []
    content_value = []

    i = 0
    for item in content_table:
        if i%2 == 0:
            content_variable.append(item)
        else:
            content_value.append(item)
        i += 1

    content_table = zip(content_variable, content_value)
    

    # Context data is the list of all uploaded files, the parent file, parent module,
    # parent packet of the results, the results itself and the selected content.
    return render(request, 'TRFC_VSLZR/results.html', {
        'uploaded_files': uploaded_files,
        'uploaded_file': up_file,
        'module': mod,
        'result': result,
        'packet': packet,
        'content': content_table
    })

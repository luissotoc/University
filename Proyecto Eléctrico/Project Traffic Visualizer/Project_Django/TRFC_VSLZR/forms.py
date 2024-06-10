import glob
import os

from django import forms
from .models import uploaded_file
from werkzeug.utils import secure_filename


#Form of uploaded files
class upload_form(forms.ModelForm):
    class Meta:
        model = uploaded_file
        #Inputs = related tag and .json file
        fields = ('tag', 'json')                #test

    #Functions that defines if the data uploaded is correct
    def clean_json(self, *args, **kwargs):

        #List of existing files
        all_files = glob.glob('TRFC_VSLZR/uploads/*.json')

        json_file = self.cleaned_data.get('json')
        json_filename = json_file.name

        ext = json_filename.rsplit(".", 1)[1]

        #Verify that uploaded file has a filename
        if json_filename == "":
            print('UPLOAD-FAIL: NOT NAME')
            raise forms.ValidationError("File must have a filename.")

        else:
            #Verify that uploaded file is a .json
            if ext.upper() != "JSON":   
                print('UPLOAD-FAIL: NOT JSON')
                raise forms.ValidationError("This is not a 'json' file.")

            else:
                #Verify that the uploaded file was not already uploaded
                for file in all_files:

                    original_name = json_filename.rsplit(".", 1)[0]

                    file_name = os.path.basename(file)
                    stored_name = file_name.rsplit(".", 1)[0]
                    

                    if original_name == stored_name:                    
                        print('UPLOAD-FAIL: CLONE')
                        raise forms.ValidationError('This file already exists.')

                #Sanitizes the file input name
                json_file.name = secure_filename(json_filename)
                return json_file



#Form of selected_packet
#class selected_packet(forms.Form):
    #Inputs = buffer_name
#    p_id_num = forms.CharField(label='p_id_num', max_length=300)

#Form of selected_buffer
#class selected_buffer(forms.Form):
    #Inputs = buffer_name
#    b_buffer_name = forms.CharField(label='b_buffer_name', max_length=300)
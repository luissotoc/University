from django import template


register = template.Library()

#Loads the previous element of a list
@register.filter
def prev(list, index):
    try:
        return list[int(index)-1]
    
    except:
        return ""

#Loads an element of another list
@register.filter
def get(list, index):
    try:
        return list[int(index)]
    
    except:
        return ""

from django.urls import path
from . import views
from .views import file_list
from django.contrib.staticfiles.urls import staticfiles_urlpatterns

# urls for each view (<url>, <view>, <name>)
urlpatterns = [
    path('', views.home, name='app-home'),
    path('test/', views.test, name='app-test'),
    path('upload/', views.upload, name='app-upload'),
    path('available/', file_list.as_view(), name='app-avail'),
    path('file/<id>/', views.file_page, name='app-file-page'),
    path('module_file/<id>/', views.module_file_page, name='app-module-file-page'),
    path('delete/<id>/', views.delete_file, name='app-delete'),
    path('results/<id_file>/<module_name>/<id_num>', views.results, name='app-results')
]

urlpatterns += staticfiles_urlpatterns()
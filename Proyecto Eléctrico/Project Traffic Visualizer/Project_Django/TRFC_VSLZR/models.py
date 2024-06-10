from django.db import models
from django.utils import timezone

# ----- UPLOADED_FILE MODEL -----
# Model of each uploaded files, includes a related tag, a file and date of upload
class uploaded_file(models.Model):
    tag = models.CharField(max_length=300)
    filename = models.CharField(max_length=300)
    date = models.DateTimeField(default=timezone.now)
    json = models.FileField(upload_to='.', )

    def __str__(self):
        return self.tag

    def delete(self, *args, **kwargs):
        self.json.delete()
        super().delete(*args, **kwargs)


# ----- MODULE MODEL -----
# Model of each module, includes a module name, total records and a related parent file
class module(models.Model):
    module_name = models.CharField(max_length=300)
    parent_file = models.ForeignKey(uploaded_file, on_delete=models.CASCADE)
    total_records = models.IntegerField()
    module_num = models.IntegerField(null=True)

    def __str__(self):
        return self.module_name

# ----- PACKET_RECORD -----
# Model of each packet record, includes a id_num, a parent capture buffer and parent file
class packet_record(models.Model):
    id_num = models.CharField(max_length=50)
    modules = models.ManyToManyField(module)
    parent_file = models.ForeignKey(uploaded_file, on_delete=models.CASCADE)
    results = models.CharField(max_length=300)

    def __str__(self):
        return self.id_num

# ----- RESULT_OUTPUT -----
# Model of the results output, this relates to a capture buffer and a packet, and includes
# the results from the capture
class result_output(models.Model):
    parent_module = models.ForeignKey(module, on_delete=models.DO_NOTHING)
    parent_file = models.ForeignKey(uploaded_file, on_delete=models.DO_NOTHING)
    parent_packet = models.ForeignKey(packet_record, on_delete=models.CASCADE)
    results = models.TextField(null=True, blank=True)
    drop = models.BooleanField(null=True)

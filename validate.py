#!/afs/csail.mit.edu/proj/courses/6.172/bin/python

'''
  Validator for 6.172 Memory Allocator, Fall 2011
'''

import os, re, subprocess, sys

TMP_DIR = "tmp/"

class ValidationError(Exception):
  def __init__(self, value):
    self.value = value
  def __str__(self):
    return repr(self.value)

# Test for overlapping blocks by storing all allocated blocks in a list, 
# and comparing all of them with a new block.
allocated_blocks = []

allocated_size = 0
max_allocated_size = 0

def process_malloc(size, return_ptr):
  # Test alignment
  if (return_ptr % 8) != 0:
    raise ValidationError("0x%x is not aligned to 8 bytes." % (return_ptr))
  
  # Test overlap
  return_ptr_end = return_ptr + size - 1
  for b in allocated_blocks:
    if (b[0] <= return_ptr_end) and (b[1] >= return_ptr):
      raise ValidationError("Payload (0x%x,0x%x) overlaps another payload (0x%x, 0x%x)" % (return_ptr, return_ptr_end, b[0], b[1]))

  allocated_blocks.append((return_ptr, return_ptr_end))
  
  global allocated_size
  global max_allocated_size
  allocated_size += size
  if allocated_size > max_allocated_size:
    max_allocated_size = allocated_size
  
def process_free(ptr):
  for b in allocated_blocks:
    if b[0] == ptr:
      global allocated_size
      allocated_size -= (b[1] - b[0] + 1)
      
      allocated_blocks.remove(b)
      return

def process_action(log_value):
  seq = log_value["seq"]
  action = log_value["action"]
  args = log_value["args"]
  
  if action == "malloc":
    process_malloc(int(args[0], 10), int(args[1], 16))
    
  elif action == "free":
    process_free(int(args[0], 16))
    
  elif action == "realloc-begin":
    process_free(int(args[0], 16))
    
  elif action == "realloc-end":
    process_malloc(int(args[1], 10), int(args[2], 16))

  else:
    raise ValidationError("Unknown action")

'''
  LogReader provides read() and next() interfaces 
    read(): get the current line
    next(): move to the next line
'''
class LogReader:
  def __init__(self, filename):
    self.f = open(filename, "r")
  
  def next(self):
    line = self.f.readline()
    if line:
      tmp = line.rstrip().split(" ")
      self.value = {
        "seq" : int(tmp[0], 10),
        "action" : tmp[1],
        "args" : tmp[2:]
        }
    else:
      self.value = None
      
    return self.value
      
  def read(self):
    return self.value

'''
  validate reads all logs, and sequentially calls process_action()
'''
def validate(files):
  readers = []
  
  # Dirty hack: If we use a single log, the file will not be sorted.
  # We store the previous actions here.
  previous_values = []
  
  for filename in files:
    reader = LogReader(filename)
    reader.next()
    readers.append(reader)

  curr_seq = 0
  while (len(readers) > 0) and (len(previous_values) == 0):
    break_for = False
    
    for reader in readers:
      val = reader.read()
      if val is None:
        # EOF
        readers.remove(reader)
        break_for = True
        break
      
      if val["seq"] == curr_seq:
        try:
          process_action(val)
        except ValidationError as error:
          print "VALIDATION ERROR: %s at seq `%s` in %s" % (error, val['seq'], reader.f.name)
          exit(1)
          
        curr_seq += 1
        reader.next()
        break_for = True
        break

    # Dirty hack: If we use a single lock, the file is not sorted.
    if len(files) == 1:
      for val in previous_values:
        if val["seq"] == curr_seq:
          try:
            process_action(val)
          except ValidationError as error:
            print "VALIDATION ERROR: %s at seq `%s` in %s" % (error, val['seq'], reader.f.name)
            exit(1)
            
          previous_values.remove(val)
          curr_seq += 1
          break_for = True
          break
          
      if not break_for:
        val = reader.next()
        while val["seq"] != curr_seq:
          previous_values.append(val)
          val = reader.next()
          
        try:
          process_action(val)
        except ValidationError as error:
          print "VALIDATION ERROR: %s at seq `%s` in %s" % (error, val['seq'], reader.f.name)
          exit(1)
            
        curr_seq += 1
        break
    
    else:
      # This is impossible.
      if not break_for:
        print "VALIDATION ERROR: Seq " + str(curr_seq) + " is missing. Please contact course staff."
        exit(1)

  print "VALIDATION SUCCESS"

def main(argv):
    if len(argv) < 2:
        print 'Usage: validate.py <commands>'
        print 'Example: validate.py ./cache-scratch-validate 12 100 8 100000'
        sys.exit(1)

    os.system('rm ' + TMP_DIR + '*.out')

    proc = subprocess.Popen(argv[1:], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (stdout, stderr) = proc.communicate()
    
    print stdout
    
    used_heap_size = int(re.findall(r'Heap size: (\d+)', stderr)[0])
    
    log_files = []
    threads = re.findall(r'Log file: (\d+)', stderr)
    for thread in threads:
      log_files.append(TMP_DIR + thread + ".out")

    if len(log_files) == 0:
      print "No log files. Did you run a `validate` version of the program?"
      sys.exit(1)

    validate(log_files)
    
    print "Peak allocated size: " + str(max_allocated_size)
    print "Used heap size: " + str(used_heap_size)
    
    free_slots = (2**10) * (40 * 12)
    score = 1.0 * max(max_allocated_size, free_slots) / max(used_heap_size, free_slots)
    print "Space utilization score: " + str(score)

if __name__ == '__main__':
    main(sys.argv)
    
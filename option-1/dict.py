# Requires dictionary file named 'million.txt', tested with https://github.com/danielmiessler/SecLists/blob/master/Passwords/Common-Credentials/10-million-password-list-top-1000000.txt

import argparse
import re
import threading
from multiprocessing import Pool

lock = threading.Lock()

def search(word):
    # create a flag called found to track if the word is found
    found = False

    # open the file to read
    with open('million.txt') as file:

        # we'll loop through every word in the dict file and use regex to search
        for line in file:
            # ^...$ regex for full match
            if re.search('^' + word  + '$', line):
                found = True
                return True
      
        # if our flag is still false, we didn't find the word in the dict
        if not found:
            return False

def delete(word):
    # let's get all of the words
    file_r = open('million.txt', 'r')    
    existing_words = file_r.readlines()
    file_r.close()

    # wait for lock
    while lock.locked():
        continue

    # acquire lock
    lock.acquire()


    # now we'll open to file to write to it, and write all lines except the word to delete
    file_w = open('million.txt', 'w')
    for existing_word in existing_words:
        if existing_word.strip('\n') != word:
             file_w.write(existing_word)

    file_w.close()
     
    #release lock
    lock.release()
    
    return

def insert(word):

    print('Inserting ' + word + ' in thread ',threading.current_thread().ident)
    # wait for lock
    while lock.locked():
        continue

    # acquire lock
    lock.acquire()

    #open file in append mode and write the new word to a new line
    file = open('million.txt', 'a')
    file.write(word + '\n')
    file.close()
    
    #release lock
    lock.release()
 
    return


def insert_many_cmd(words):
    threads = []
    for word in words:
        threads.append(threading.Thread(target=insert, args=[word]))
        threads[-1].start()
    for t in threads:
        t.join()

def search_cmd(word):
    print('Searching for', word)
    if search(word):
        print(word, 'is in the dictionary.')

    else:
        print(word, 'is NOT in the dictionary.')


def insert_cmd(word):
    # check to see if word is NOT already in dictionary
    if not search(word):
        # word is NOT in dictionary, let's insert it
        print('Inserting', word)

        insert(word)

    else:
        # word is in dictionary...
        print(word, 'is already in the dictionary.')
        return

def delete_cmd(word):
    # check to see if word is in dictionary before we attempt to delete it
    if search(word):
        # word is in dictionary, let's delete it
        print('Deleting', word)
        
        delete(word)
        
    else:
        # word isn't in dictionary...
        print(word, "isn't even in the dictionary, can't delete it...")
        return


def main():
    parser = argparse.ArgumentParser(description='Best Dictionary Ever')
    parser.add_argument('--insert', nargs=1, metavar='word')
    parser.add_argument('--search', nargs=1, metavar='word')
    parser.add_argument('--delete', nargs=1, metavar='word')
    parser.add_argument('--insertmany', nargs='*', metavar='words')   

    args = parser.parse_args()

    if args.insert:
        insert_cmd(args.insert[0])
    elif args.search:
        search_cmd(args.search[0])
    elif args.delete:
        delete_cmd(args.delete[0])
    elif args.insertmany:
        insert_many_cmd(args.insertmany)


if __name__ == "__main__":
    main()


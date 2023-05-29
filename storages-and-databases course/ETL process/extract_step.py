from datetime import datetime

from general_utils import write_log


def extract_from_couch(db_couch, key_str):
    write_log('Started extraction of data from CouchDB for provided date:' + str(key_str))
    try:
       couch_records_rec = db_couch.view('lib_view/by_date', key = key_str)
       write_log('Records data was extracted from CouchDB: ' + str(len(couch_records_rec)) + ' records')
    except Exception as e:
        write_log('CouchDB extraction records data error: ' + str(e))
    try:
       couch_records_book = db_couch.view('lib_view/all_books')
       write_log('Books data was extracted from CouchDB: ' + str(len(couch_records_book)) + ' records')
    except Exception as e:
        write_log('CouchDB extraction books data error: ' + str(e))
    try:
       couch_records_people = db_couch.view('lib_view/all_people')
       write_log('People data was extracted from CouchDB: ' + str(len(couch_records_people)) + ' records')
    except Exception as e:
        write_log('CouchDB extraction people data error: ' + str(e))
    couch_data_rec=[records.value for records in couch_records_rec]
    couch_data_book = [records.value for records in couch_records_book]
    couch_data_people = [records.value for records in couch_records_people]
    write_log('Finished extraction of data from CouchDB\n')
    return couch_data_rec, couch_data_book, couch_data_people


def extract_from_mysql(mysql_cursor, key_str):
    write_log('Started extraction of data from MySQL for provided date:' + str(key_str))
    mysql_records_rec =[]
    mysql_records_book = []
    mysql_records_people = []
    try:
       mysql_cursor.execute("SELECT * FROM records WHERE date_of_issue LIKE %s", [key_str+'%'])
       mysql_records_rec = mysql_cursor.fetchall()
       write_log('Records data was extracted from MySQL: ' + str(len(mysql_records_rec)) + ' records')
    except Exception as e:
        write_log('MySQL extraction records data error: ' + str(e))
    try:
       mysql_cursor.execute("SELECT * FROM books")
       mysql_records_book = mysql_cursor.fetchall()
       write_log('Books data was extracted from MySQL: ' + str(len(mysql_records_book)) + ' records')
    except Exception as e:
        write_log('MySQL extraction books data error: ' + str(e))
    try:
       mysql_cursor.execute("SELECT * FROM readers")
       mysql_records_people = mysql_cursor.fetchall()
       write_log('Readers data was extracted from MySQL: ' + str(len(mysql_records_people)) + ' records\n')
    except Exception as e:
        write_log('MySQL extraction readers data error: ' + str(e)+'\n')
    return mysql_records_rec, mysql_records_book, mysql_records_people


def extract_data_from_dbs(key_str, db_handlers):
    temp_key = datetime.strptime(key_str, '%d/%m/%Y')
    # specify the format of date for CouchDB
    key_for_couch = temp_key.strftime('%d.%m.%Y')
    # specify the format of date for MySQL
    key_for_mysql = temp_key.strftime('%b %d %Y')
    db_couch = db_handlers[0]
    mysql_cursor = db_handlers[1]
    couch_data_rec, couch_data_book, couch_data_people =  extract_from_couch(db_couch, key_for_couch)
    mysql_records_rec, mysql_records_book, mysql_records_people = extract_from_mysql(mysql_cursor, key_for_mysql)
    return [couch_data_rec, couch_data_book, couch_data_people, mysql_records_rec, mysql_records_book, mysql_records_people]
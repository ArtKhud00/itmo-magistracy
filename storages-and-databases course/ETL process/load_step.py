import pandas as pd

from datetime import datetime

from general_utils import write_log


def load_data_to_postgresql(db_handlers, df_db_books, df_db_people, df_db_records):
    pg_cursor = db_handlers[2]
    pg_conn = db_handlers[3]
    write_log('Starting loading data into PostgreSQL database')
    write_log('Loading books in PostgreSQL')
    for book in df_db_books:
        try:
            temp1 = book['book_num_1']
            temp2 = book['book_num_2']
            if(pd.isnull(temp1)):
                temp1 = 0
            if(pd.isnull(temp2)):
                temp2 = 0
            temp1 = (int)(temp1)
            temp2 = (int)(temp2)
            # Добавить ON CONFLICT
            pg_cursor.execute("INSERT INTO books (id_book,title_book,author_book, type_book, year_book, library1_code, library2_code) " +\
                              "VALUES (%s, %s, %s, %s, %s, %s, %s) " +\
                              "ON CONFLICT(id_book) " +\
                              "DO UPDATE SET (library1_code, library2_code)=(EXCLUDED.library1_code, EXCLUDED.library2_code)",
                              (book['id'],book['title'],book['author'],book['type'],book['year'],temp1,temp2)
                              )
            pg_conn.commit()
        except Exception as e:
            print('Error ' + str(e))
            write_log('Error loading books in PostgreSQL: ' + str(e))
    write_log('Finished loading books')
    write_log('Starting loading people in PostgreSQL')
    for person in df_db_people:
        try:
            temp1 = person['library_card_1']
            temp2 = person['library_card_2']
            if(pd.isnull(temp1)):
                temp1 = 0
            if(pd.isnull(temp2)):
                temp2 = 0
            temp1 = (int)(temp1)
            temp2 = (int)(temp2)
            pg_cursor.execute("INSERT INTO people (id,surname_reader, name_reader, patronymic_reader, date_of_birth, library1_code, library2_code) " +\
                              "VALUES (%s, %s, %s, %s, %s, %s, %s) " +\
                              "ON CONFLICT (id) " +\
                              "DO UPDATE SET (library1_code, library2_code)=(EXCLUDED.library1_code, EXCLUDED.library2_code) ",
                              (person['id'],
                               person['surname'],
                               person['name'],
                               person['patronymic'],
                               person['date_of_birth'],
                               temp1, temp2)
                              )
            pg_conn.commit()    
        except Exception as e:
            print('Error '+str(e))
            write_log('Error loading people in PostgreSQL ' + str(e))       
    try:
        pg_cursor.executemany("INSERT INTO records (id_reader, id_book, date_of_issue, date_of_return, id_library) " +\
                              "VALUES (%s, %s, %s, %s, %s)", df_db_records)
        pg_conn.commit()
    except Exception as e:
        write_log('Error loading records in PostgreSQL ' + str(e))
            

def load_data_to_clickhouse(db_handlers,date_of_extraction):
    pg_cursor = db_handlers[2]
    ch_conn = db_handlers[4]
    temp_key = datetime.strptime(date_of_extraction, '%d/%m/%Y')
    key_for_postgres = temp_key.strftime('%Y-%m-%d')
    pg_cursor.execute("SELECT id_rec, id_reader,id_book, (SELECT type_book from books where id_book = records.id_book), date_of_issue, date_of_return, id_library " +\
                      "FROM records WHERE CAST(date_of_issue as DATE) = " 
                      + str("'"+key_for_postgres+"'"))
    pg_data = pg_cursor.fetchall()
    ch_data = [(int(row[0]),int(row[1]),int(row[2]),row[3],row[4],row[5],int(row[6])) for row in pg_data]
    ch_conn.execute("INSERT INTO recdata2 " +\
                    "(id_rec, id_reader, id_book, type_book, date_of_issue, date_of_return, lib_id) VALUES", ch_data)
    
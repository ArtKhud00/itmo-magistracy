import pandas as pd
import numpy as np

from general_utils import (write_log, transform_people_fields, transform_book_fields,
                           transform_date_fields)


def transform_couch_data(couch_data_rec, couch_data_book, couch_data_people, df_db_people, df_db_books):
    ## ЭТАП Transform
    write_log('Started transforming of CouchDB data')
    df_extr_rec=pd.DataFrame.from_dict(couch_data_rec)
    # извлечение полей книги
    df_extr_books = pd.DataFrame.from_dict(couch_data_book)
    # извлечение полей людей
    df_extr_people = pd.DataFrame.from_dict(couch_data_people)
    string_format = '%d.%m.%Y'
    write_log('Transforming people fields in records table by replacing to values from common people database')
    new_db_people, df_extr_rec1 = transform_people_fields(df_extr_people,df_extr_rec,string_format,df_db_people,1)
    write_log('Transforming book fields in records table by replacing to values from common books database')
    new_db_books, df_extr_rec1 = transform_book_fields(df_extr_books, df_extr_rec1, df_db_books, 1)
    write_log('Transforming date fields by converting it to specified format')
    df_extr_rec1 = transform_date_fields(df_extr_rec1, 1)
    # specify the library, for CouchDB library is 1
    type_of_lib =np.array( [1] * len(df_extr_rec1))
    write_log('Adding information about library')
    d={'type_of_lib':type_of_lib}
    df2= pd.DataFrame(data=d)
    df_extr_rec1 = pd.concat([df_extr_rec1,df2],axis=1)
    write_log('Finished transforming CouchDB data\n')
    return new_db_books, new_db_people, df_extr_rec1
    

def transform_mysql_data(mysql_data_rec, mysql_data_book, mysql_data_people, df_db_people, df_db_books):
    # Load extracted from SQL database data to DataFrames
    write_log('Started transforming of MySQL data')
    df_extr_rec = pd.DataFrame(mysql_data_rec, columns = ['id_rec','reader','book','date_of_issue','date_of_return'])
    df_extr_books = pd.DataFrame(mysql_data_book, columns=['lib_book_id','author','title', 'genre', 'year'])
    df_extr_people = pd.DataFrame(mysql_data_people, columns = ['library_card','reader_surname','reader_name',
                                                                'reader_patronymic','date_of_birth'])        
    # specify the date format which is used in MySQL DB
    string_format = '%b %d %Y'
    write_log('Transforming people fields in records table by replacing to values from common people database')
    new_db_people, df_extr_rec = transform_people_fields(df_extr_people,df_extr_rec,string_format,df_db_people,2)
    write_log('Transforming book fields in records table by replacing to values from common books database')
    new_db_books, df_extr_rec = transform_book_fields(df_extr_books, df_extr_rec, df_db_books, 2)
    write_log('Transforming date fields by converting it to specified format')
    df_extr_rec = transform_date_fields(df_extr_rec, 2)
    write_log('Adding information about library')
    # specify the library, for MySQL DB library is 2
    type_of_lib =np.array( [2] * len(df_extr_rec))
    d={'type_of_lib':type_of_lib}
    df2= pd.DataFrame(data=d)
    df_extr_rec3 = pd.concat([df_extr_rec,df2],axis=1)
    write_log('Finished transforming MySQL data\n')
    return new_db_books, new_db_people, df_extr_rec3
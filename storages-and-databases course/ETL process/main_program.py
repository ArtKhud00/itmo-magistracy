import couchdb
import psycopg2
import mysql.connector
import pandas as pd

from clickhouse_driver import Client

from general_utils import write_log
from extract_step import extract_data_from_dbs
from transform_step import transform_couch_data, transform_mysql_data
from load_step import load_data_to_postgresql, load_data_to_clickhouse

# db_handlers - список баз, их дескрипторов
def start_ETL(db_handlers, df_db_books, df_db_people, date_of_extraction):
    # Extract
    extracted_data = extract_data_from_dbs(date_of_extraction, db_handlers)
    # Transform
    df_db_books, df_db_people, df_extr_rec1 = transform_couch_data(extracted_data[0], extracted_data[1], extracted_data[2], 
                                                                  df_db_people, df_db_books)
    df_db_books, df_db_people, df_extr_rec2 = transform_mysql_data(extracted_data[3], extracted_data[4], extracted_data[5], 
                                                                  df_db_people, df_db_books)
    db_books_to_load = df_db_books.to_dict('records')
    db_people_to_load = df_db_people.to_dict('records')
    df_extr_rec2 = df_extr_rec2.drop(['id_rec'],axis = 1)
    db_records_to_load = list(df_extr_rec1.itertuples(index = False, name = None)) + list(df_extr_rec2.itertuples(index = False, name = None))  
    # Load 
    load_data_to_postgresql(db_handlers, db_books_to_load, db_people_to_load, db_records_to_load)
    load_data_to_clickhouse(db_handlers,date_of_extraction)
    return df_db_books, df_db_people, df_extr_rec1, df_extr_rec2
    





if __name__ == "__main__":
    
    # Estabilishing connection with databases
    db_handlers = []
    try: 
        write_log('Trying to connect to CouchDB')
        couch = couchdb.Server('http://admin:artem@localhost:5984/')
        db_couch = couch['library_petrogradskaya']
        db_handlers.append(db_couch)
        write_log('Successfully connected to CouchDB')
    except Exception as e:
         write_log('CouchDB connection error: ' + str(e))
    try:
        write_log('Trying to connect to MySQL')
        mysql_conn = mysql.connector.connect(
          host="localhost",
          user="root",
          password="artemroot",
          database="liibrary_nevskaya"
        )
        mysql_cursor = mysql_conn.cursor()
        db_handlers.append(mysql_cursor)
        write_log('Successfully connected to MySQL')
    except Exception as e:
        write_log('MySQL connection error ' + str(e))
    try:
        write_log('Trying to connect to PostgreSQL')
        pg_conn = psycopg2.connect(
          host="localhost",
          database="ImplementETL",
          user="postgres",
          password="artem",
          port = "5432"
        )
        pg_cursor = pg_conn.cursor()
        db_handlers.append(pg_cursor)
        db_handlers.append(pg_conn)
        write_log('Successfully connected to PostgreSQL\n')
    except Exception as e:
        write_log('PostgreSQL connection error ' + str(e))
    try:
        write_log('Trying to connect to ClickHouse\n')
        ch_conn = Client('localhost')
        write_log('Successfully connected to ClickHouse')
        db_handlers.append(ch_conn)
    except Exception as e:
        write_log('PostgreSQL connection error ' + str(e))
    
    # Create tables(DataFrames) for people and books
    df_db_books = pd.DataFrame(columns=['id','author','title','year','type','book_num_1','book_num_2'])
    df_db_people = pd.DataFrame(columns=['id','surname','name','patronymic','date_of_birth','library_card_1','library_card_2'])
    # Start ETL process
    df_db_books, df_db_people, df_extr_rec, df_extr_rec2 = start_ETL(db_handlers, df_db_books,df_db_people, '15/03/2023')
    pg_conn.close()
    mysql_conn.close()
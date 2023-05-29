import pandas as pd

from datetime import datetime, timedelta



def write_log(message):
    timestamp_format = '%Y-%h-%d-%H:%M:%S'
    now = datetime.now()
    timestamp = now.strftime(timestamp_format)
    with open("logfile.txt","a") as f:
        f.write(timestamp+', '+ message +'\n')



def transform_people_fields(df_extr_people, df_extr_rec, date_format, df_db_people, source):
    requiered_format = '%Y-%m-%d'
    # Обработка данных из CouchDB
    if(source == 1):
    # Извлечение из поля name три отдельных поля Фамилия Имя Отчество
      write_log('Divide field name to three individual fields: surname, name, patronymic')
      surnames = []
      names = []
      patronymics = []
      for st in df_extr_people['name']:
          cut = st.split(sep = ' ')
          surnames.append(cut[0])
          names.append(cut[1])
          if(len(cut) == 3):
              patronymics.append(cut[2])
          else:
              patronymics.append('')
      # Делаем замену одного единого поля name на три раздельных поля
      d={'reader_surname':surnames, 'reader_name': names, 'reader_patronymic':patronymics}
      df1 = pd.DataFrame(data=d)
      df_extr_people = df_extr_people.drop(['name'],axis=1)
      df_extr_people = pd.concat([df1,df_extr_people],axis=1)
      
    # Перевод даты рождения в нужный формат
    write_log('Converting date of birht to provided format '+ str(requiered_format))
    convert_date1 = []
    time = df_extr_people['date_of_birth']
    for i in range (len(time)):
        temp = datetime.strptime(time[i], date_format)
        convert_date1.append(temp.strftime(requiered_format))
    # Перебор записей
    for i in range (len(df_extr_people)):
        # Поиск, есть ли человек в базе читателей из всех библиотек
        write_log('Checking if person is included to common people database')        
        temp = (df_db_people[(df_db_people['surname'] == df_extr_people['reader_surname'][i])
                            & (df_db_people['name'] == df_extr_people['reader_name'][i])
                            & (df_db_people['patronymic'] == df_extr_people['reader_patronymic'][i])
                            & (df_db_people['date_of_birth'] == convert_date1[i])] )
        if(len(temp) == 0):
          # Если человек не был найден, то добавляем его в базу
              write_log('Person is not included, adding to common people database')
              list_to_add = []
              new_id = len(df_db_people) + 1
              if(source == 1):
                temp_df = { 
                            'id' : new_id,
                            'surname' : df_extr_people['reader_surname'][i],
                            'name' : df_extr_people['reader_name'][i],
                            'patronymic' : df_extr_people['reader_patronymic'][i],
                            'date_of_birth' : convert_date1[i],
                            'library_card_1' : df_extr_people['library_card'][i]
                          }
              if (source == 2):
                temp_df = { 
                            'id' : new_id,
                            'surname' : df_extr_people['reader_surname'][i],
                            'name' : df_extr_people['reader_name'][i],
                            'patronymic' : df_extr_people['reader_patronymic'][i],
                            'date_of_birth' : convert_date1[i],
                            'library_card_2' : df_extr_people['library_card'][i]
                          }
              list_to_add.append(temp_df)
              temp_df = pd.DataFrame(list_to_add)
              df_db_people = pd.concat([df_db_people,temp_df])
              df_db_people=df_db_people.reset_index(drop=True)
        else:
              write_log('Person is included, checking information about library card')
              # Если человек был найден, проверяем номер читательского билета для рассматриваемой библиотеки
              if(source == 1):
                if(pd.isnull(df_db_people.loc[temp.index[0],'library_card_1'])):
                    df_db_people['library_card_1'][temp.index[0]] = df_extr_people['library_card'][i]
                    write_log('Information about library card in 1 library has been added')
              if(source == 2):
                if(pd.isnull(df_db_people.loc[temp.index[0],'library_card_2'])):
                      df_db_people['library_card_2'][temp.index[0]] = df_extr_people['library_card'][i]
                      write_log('Information about library card in 2 library has been added')
    
    for i in range (len(df_extr_rec)):
           write_log('Changing a value in reader field by id from common people database')
           if(source == 1):
             temp = (df_db_people[(df_db_people['library_card_1']==df_extr_rec['reader'][i])])
           if(source == 2):
             temp = (df_db_people[(df_db_people['library_card_2']==df_extr_rec['reader'][i])])
           if(len(temp)>0):
               df_extr_rec.at[i,'reader'] = temp['id'][temp.index[0]]     
        
    write_log('Finished transforming people fields\n')          
    return df_db_people, df_extr_rec  
    


def transform_book_fields(df_extr_books, df_extr_rec, df_db_books, source):
    # Перебор записей
    #changed_id = []
    for i in range (len(df_extr_books)):

        # Поиск, есть ли человек в базе читателей из всех библиотек
        write_log('Checking if book is included to common books database')
        temp = (df_db_books[(df_db_books['author'] == df_extr_books['author'][i])
                            & (df_db_books['title'] == df_extr_books['title'][i])
                            & (df_db_books['year'] == df_extr_books['year'][i])
                            & (df_db_books['type'] == df_extr_books['genre'][i])] )
        if(len(temp) == 0):
        # Если книга не была найдена, то добавляем ее в базу
            write_log('Book is not included, adding to common books database')
            list_to_add = []
            new_id = len(df_db_books) + 1
            if(source == 1):
              temp_df = { 
                          'id' : new_id,
                          'author' : df_extr_books['author'][i],
                          'title' : df_extr_books['title'][i],
                          'year' : df_extr_books['year'][i],
                          'type' : df_extr_books['genre'][i],
                          'book_num_1' : df_extr_books['lib_book_id'][i],
                        }
            if(source == 2): 
              temp_df = { 
                          'id' : new_id,
                          'author' : df_extr_books['author'][i],
                          'title' : df_extr_books['title'][i],
                          'year' : df_extr_books['year'][i],
                          'type' : df_extr_books['genre'][i],
                          'book_num_2' : df_extr_books['lib_book_id'][i]
                        }
            list_to_add.append(temp_df)
            temp_df = pd.DataFrame(list_to_add)
            df_db_books = pd.concat([df_db_books,temp_df])
            df_db_books=df_db_books.reset_index(drop=True)
        else:
            write_log('Book is included, checking information about book num')
            # Если человек был найден, проверяем номер читательского билета для рассматриваемой библиотеки
            if(source == 1):
              if(pd.isnull(df_db_books.loc[temp.index[0],'book_num_1'])):
                  df_db_books['book_num_1'][temp.index[0]] = df_extr_books['lib_book_id'][i]
                  write_log('Information about book num in 1 library has been added')
            if(source == 2):
              if(pd.isnull(df_db_books.loc[temp.index[0],'book_num_2'])):
                  df_db_books['book_num_2'][temp.index[0]] = df_extr_books['lib_book_id'][i]
                  write_log('Information about book num in 2 library has been added')
           
    
    for i in range (len(df_extr_rec)):
         write_log('Changing a value in book field by id from common books database')
         if(source == 1):
           temp = (df_db_books[(df_db_books['book_num_1']==df_extr_rec['book'][i])])
         if(source == 2):
           temp = (df_db_books[(df_db_books['book_num_2']==df_extr_rec['book'][i])])
            #print("temp= ",temp)
         if(len(temp)>0):
             df_extr_rec.at[i,'book'] = temp['id'][temp.index[0]]
    write_log('Finished transforming people fields\n')
    return df_db_books, df_extr_rec


def transform_date_fields(df_extr_rec, source):
    requiered_format = '%Y-%m-%d %H:%M:%S'
    if(source == 1):
        convert_date1=[]
        convert_date2=[]
        date = df_extr_rec['date_of_issue']
        time = df_extr_rec['time_of_taking']
        delta = df_extr_rec['duration_of_using']
        for i in range (len(time)):
            temp = datetime.strptime(date[i]+" "+time[i], '%d.%m.%Y %H:%M')
            convert_date1.append(temp.strftime(requiered_format)) 
            convert_date2.append((temp + timedelta(days=(int)(delta[i]))).strftime(requiered_format))
        d={'date_of_issue':convert_date1,'date_of_return':convert_date2}
        df1 = pd.DataFrame(data=d)
        df_extr_rec=df_extr_rec.drop(['date_of_issue','time_of_taking','duration_of_using'],axis=1)
        df_extr_rec = pd.concat([df_extr_rec,df1],axis=1)
        return df_extr_rec
    if(source == 2):
        convert_date1=[]
        convert_date2=[]
        date1 = df_extr_rec['date_of_issue']
        date2 = df_extr_rec['date_of_return']
        for i in range (len(date1)):
            temp1 = datetime.strptime(date1[i], '%b %d %Y %H:%M')
            temp2 = datetime.strptime(date2[i], '%b %d %Y %H:%M')
            convert_date1.append(temp1.strftime(requiered_format))
            convert_date2.append(temp2.strftime(requiered_format))
        d={'date_of_issue':convert_date1,'date_of_return':convert_date2}
        df1 = pd.DataFrame(data=d)
        df_extr_rec=df_extr_rec.drop(['date_of_issue','date_of_return'],axis=1)
        df_extr_rec = pd.concat([df_extr_rec,df1],axis=1)
        return df_extr_rec
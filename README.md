# Mib-Generator
Generates MIB file taking datas from .ods file
MIB Generator

Kod aşamaları

1) Dosyayı system() komutu ile ods ten csv ye çevrildi. (system() komutu yerine alternatif bulunmalı)
2) Oluşan .csv dosyasını tekte okumakta ve BUFFER_READ char arrayine depolanmakta.
3) Herhangi bir error durumunda  free_everything_and_terminate() fonksiyonu çağırılıp program terminate edilmektedir.
4) Parser fonksiyon sırası ile  miblerin içine linked list DS(Data Structure) yapısı kullanarak grupları depolamakta. Aynı şekilde gruplar objeleri (table – scalar) ve objelerde leafleri depolamaktadırlar.

Not: Her şey fonksiyonlar aracılığı ile yapılmıştır. Ve her fonksiyon ayrıca olabildiğince test edilmiştir.

Not2: Mümkün olduğunca low level coding kullanılmıştır.	

Not3: Boş satır kontrolleri, leaf sayısının belirtilene uyup uymadığı gibi kontroller yapılmıştır fakat hala eksik olması ihtimaline karşın mib için gelen input exel dosyasının özenle hazırlanmış olması gerekmektedir. Sonuçta GUI ile tasarlanmamış bir exel olcağından dolayı olası hatalara hazır olmak gereklidir.

Genel structure yapısı

Kodun çalışması için 1 makefile, 1 düzgün yazılmış bir .ods file ve kodumuz yeterlidir.

1) make yazılır.
2) ./program_adı -i inputfile.ods yazılır ve çalıştırılır.
Not: Valgrind için make valgrind yazmanız yeterlidir.
Not 2: make clean ile .csv dosyası silinebilir.

 
Eksikler

1) System komutu değişmeli çünkü dosyayı ilk açılışta oluşturmamakta. 2. kez execute etmek gerekmekte.

2) Create mib file fonksiyonu yazılması lazım.

Dosya Okuma Sistemi

Bilindiği üzere genelde dosya okuma işlemleri yoğun olduğu durumda hata payını azaltmak ve modularity’ yi arttırmak adına dosya okumaya özel kütüphane yazılır. Burda da görüleceği özere kendi ufak kütüphanemi yazmış bulundum. BUFFER_READ arrayini o fonksiyonlar ile okudum.






Genel structure yapısı:


Görüldüğü üzere iç içe bir yapı var ve her biri birbirini linked list olarak depolamakta. Herhangi bir random acccess gerekmesi durumunda mecburen tek tek traverse etmen gerekcek. (Bunu ben seçmedim benden istendi)

Structure’larda traverse yapmak (dolanmak) istersen:

Burada ki kodda görceğin üzere (source kodda mevcut), her birinin iterator’ü aracılığıyla iç içe while kullanarak ilerlenebilmektedir.


Exel File Yapısı

Exel file (.ods) dosyası da benim taradımca oluşturulmuştur, belli bir düzeni yoktur. Dolduracak kişinin kolay doldurabilmesi şartı ile bazı değişiklikler yapılması mümkündür.
Yapı genelden özele : MIB → GROUP → TABLE/SCALAR → LEAF şeklindedir.

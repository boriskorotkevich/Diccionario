#include <array>
#include <algorithm>
#include <bitset>
#include <memory>
#include <iostream>
#include <type_traits>
#include <utility>
#include <stddef.h>
#include <stdint.h>

namespace aux {
   template<typename T, size_t N>
   std::array<std::unique_ptr<T>, N> copia_valores(const std::array<std::unique_ptr<T>, N>& valores) {
      std::array<std::unique_ptr<T>, N> res;
      std::transform(valores.begin( ), valores.end( ), res.begin( ), [](const auto& actual) {
         return (actual != nullptr ? std::make_unique<T>(*actual) : nullptr);
      });
      return res;
   }

   template<typename T, size_t N>
   bool compara_valores(const std::array<std::unique_ptr<T>, N>& valores1, const std::array<std::unique_ptr<T>, N>& valores2) {
      return std::equal(valores1.begin( ), valores1.end( ), valores2.begin( ), [](auto& v1, auto& v2) {
         return ((v1 == nullptr) == (v2 == nullptr)) && (v1 == nullptr || *v1 == *v2);
      });
   }
}

template<typename T, size_t N> requires(N <= 256)
class representacion1 {
   uint16_t tam;
   std::array<uint8_t, N> claves;
   std::array<std::unique_ptr<T>, N> valores;

public:
   representacion1( )
   : tam(0), claves{ } {
   }

   representacion1(const representacion1& otro)
   : tam(otro.tam), claves(otro.claves), valores(aux::copia_valores(otro.valores)) {
   }

   representacion1(representacion1&& otro)
   : tam(otro.tam), claves(otro.claves), valores(std::move(otro.valores)) {
      otro.tam = 0, otro.claves = { };
   }

   representacion1& operator=(representacion1 otro) {
      this->~representacion1( );
      return *(new(this) representacion1(std::move(otro)));
   }

   bool con_capacidad( ) const {
      return N == 256 || tam < N;
   }

   bool existe(uint8_t c) const {
      int i = posicion(c);
      return i != tam && claves[i] == c;
   }

   const T* consulta(uint8_t c) const {
      return valores[posicion(c)].get( );
   }

   void agrega(uint8_t c, const T& v) {
      int i = posicion(c);
      if (i == tam || claves[i] != c) {
         tam += 1;
         std::move_backward(&claves[i], &claves[tam - 1], &claves[tam]);
         std::move_backward(&valores[i], &valores[tam - 1], &valores[tam]);
      }
      claves[i] = c, valores[i] = std::make_unique<T>(v);
   }

   bool operator==(const representacion1& otro) const {
      return tam == otro.tam && claves == otro.claves && aux::compara_valores(valores, otro.valores);
   }

private:
   int posicion(uint8_t c) const {
      return std::lower_bound(claves.begin( ), claves.begin( ) + tam, c) - claves.begin( );
   }
};

template<typename T, size_t N> requires(N <= 256)
class representacion2 {
   std::bitset<256> bitset;
   std::array<std::unique_ptr<T>, N> valores;

public:
   representacion2( ) = default;

   representacion2(const representacion2& otro)
   : bitset(otro.bitset), valores(aux::copia_valores(otro.valores)) {
   }

   representacion2(representacion2&& otro)
   : bitset(otro.bitset), valores(std::move(otro.valores)) {
      otro.bitset = { };
   }

   representacion2& operator=(representacion2 otro) {
      this->~representacion2( );
      return *(new(this) representacion2(std::move(otro)));
   }

   bool con_capacidad( ) const {
      return N == 256 || valores[N - 1] == nullptr;
   }

   bool existe(uint8_t c) const {
      return bitset[c];
   }

   const T* consulta(uint8_t c) const {
      return valores[posicion(c)].get( );
   }

   void agrega(uint8_t c, const T& v) {
      int i = posicion(c);
      if (!bitset[c]) {
         bitset[c] = true;
         std::move_backward(&valores[i], &valores[N - 1], &valores[N]);
      }
      valores[i] = std::make_unique<T>(v);
   }

   bool operator==(const representacion2& otro) const {
      return bitset == otro.bitset && aux::compara_valores(valores, otro.valores);
   }

private:
   int posicion(uint8_t c) const {
      return (std::bitset<256>(bitset) << (256 - c)).count( );
   }
};

template<typename T>
class representacion3 {
   std::array<std::unique_ptr<T>, 256> valores;

public:
   representacion3( ) = default;

   representacion3(const representacion3& otro)
   : valores(aux::copia_valores(otro.valores)) {
   }

   representacion3(representacion3&& otro)
   : valores(std::move(otro.valores)) {
   }

   representacion3& operator=(representacion3 otro) {
      this->~representacion3( );
      return *(new(this) representacion3(std::move(otro)));
   }

   bool con_capacidad( ) const {
      return true;
   }

   bool existe(uint8_t c) const {
      return (valores[c] != nullptr);
   }

   const T* consulta(uint8_t c) const {
      return valores[c].get( );
   }

   void agrega(uint8_t c, const T& v) {
      valores[c] = std::make_unique<T>(v);
   }

   bool operator==(const representacion3& otro) const {
      return aux::compara_valores(valores, otro.valores);
   }
};

// especializaciones para void

template<size_t N> requires(N <= 256)
class representacion1<void, N> {
   uint16_t tam = 0;
   std::array<uint8_t, N> claves = { };

public:
   bool con_capacidad( ) const {
      return N == 256 || tam < N;
   }

   bool existe(uint8_t c) const {
      int i = posicion(c);
      return i != tam && claves[i] == c;
   }

   void agrega(uint8_t c) {
      int i = posicion(c);
      if (i == tam || claves[i] != c) {
         tam += 1;
         std::move_backward(&claves[i], &claves[tam - 1], &claves[tam]);
      }
      claves[i] = c;
   }

   bool operator==(const representacion1& otro) const = default;

private:
   int posicion(uint8_t c) const {
      return std::lower_bound(claves.begin( ), claves.begin( ) + tam, c) - claves.begin( );
   }
};

template<size_t N> requires(N <= 256)
class representacion2<void, N> {
   std::bitset<256> bitset;

public:
   bool con_capacidad( ) const {
      return true;
   }

   bool existe(uint8_t c) const {
      return bitset[c];
   }

   void agrega(uint8_t c) {
      bitset[c] = true;
   }

   bool operator==(const representacion2& otro) const = default;
};

template<>
class representacion3<void> {
   std::bitset<256> bitset;

public:
   bool con_capacidad( ) const {
      return true;
   }

   bool existe(uint8_t c) const {
      return bitset[c];
   }

   void agrega(uint8_t c) {
      bitset[c] = true;
   }

   bool operator==(const representacion3& otro) const = default;
};

// selector de tabla

template<typename T, size_t N>
using tabla = std::conditional_t<
   sizeof(representacion1<T, N>) == std::min({ sizeof(representacion1<T, N>), sizeof(representacion2<T, N>), sizeof(representacion3<T>) }), representacion1<T, N>, std::conditional_t<
      sizeof(representacion2<T, N>) == std::min({ sizeof(representacion1<T, N>), sizeof(representacion2<T, N>), sizeof(representacion3<T>) }), representacion2<T, N>, representacion3<T>
   >
>;


//------------------------------------------------------------------------------------------
template<typename T> 
class tabla_interfaz{ 
public: 
    virtual ~tabla_interfaz( ) = default; 
    virtual bool con_capacidad( ) const = 0; 
    virtual bool existe(uint8_t clave) const = 0; 
    virtual const T* consulta(uint8_t clave) const = 0; 
    virtual void agrega(uint8_t clave, const T& valor) = 0; 
    virtual bool operator==(const tabla_interfaz<T>& t) const = 0; 
    virtual std::unique_ptr<tabla_interfaz<T>> clona_mayor( ) const = 0; 
}; 
 
template<> 
class tabla_interfaz<void>{ 
public: 
    virtual ~tabla_interfaz( ) = default; 
    virtual bool con_capacidad( ) const = 0; 
    virtual bool existe(uint8_t clave) const = 0; 
    virtual void agrega(uint8_t clave) = 0; 
    virtual bool operator==(const tabla_interfaz<void>& t) const = 0; 
    virtual std::unique_ptr<tabla_interfaz<void>> clona_mayor( ) const = 0; 
}; 
 
template<typename T,size_t N> requires(N <= 256) 
class tabla_polimorfica : public tabla_interfaz<T>{ 
    tabla<T,N> temp; 
 
public: 
    bool con_capacidad() const override{ 
        return temp.con_capacidad(); 
    } 
 
    bool existe(uint8_t clave) const override{ 
        return temp.existe(clave); 
    } 
 
    const T* consulta(uint8_t clave) const override{ 
        return temp.consulta(clave); 
    } 
 
    void agrega(uint8_t clave, const T& valor) { 
        temp.agrega(clave,valor); 
    } 
 
    bool operator==(const tabla_interfaz<T>& t) const override { 
       for(int i = 0; i<256; i++){ 
          if((t.existe(i) && temp.existe(i) && *(temp.consulta(i)) == *(t.consulta(i))) || (!t.existe(i) && !temp.existe(i)) ){ 
             continue; 
          } 
          return false; 
       } 
       return true; 
    } 
 
    std::unique_ptr<tabla_interfaz<T>> clona_mayor( ) const override { 
        auto t = std::make_unique<tabla_polimorfica<T, std::min(std::max(N + 1, 2 * N), size_t(256))>>(); 
        for(int i = 0; i<256; i++){ 
          if(temp.existe(i)){ 
             t->agrega(i,*(temp.consulta(i))); 
          } 
       } 
        return t; 
    } 
}; 
 
template<size_t N> requires(N <= 256) 
class tabla_polimorfica<void,N> : public tabla_interfaz<void>{ 
    tabla<void,N> temp; 
 
public: 
    bool con_capacidad() const override{ 
        return temp.con_capacidad(); 
    } 
 
    bool existe(uint8_t clave) const override{ 
        return temp.existe(clave); 
    } 
 
    void agrega(uint8_t clave) { 
        temp.agrega(clave); 
    } 
 
    bool operator==(const tabla_interfaz<void>& t) const override { 
       for(int i = 0; i<256; i++){ 
          if((t.existe(i) && temp.existe(i)) || (!t.existe(i) && !temp.existe(i))){ 
             continue; 
          } 
          return false; 
       } 
       return true; 
    } 
 
    std::unique_ptr<tabla_interfaz<void>> clona_mayor( ) const override { 
        auto t = std::make_unique<tabla_polimorfica<void, std::min(std::max(N + 1, 2 * N), size_t(256))>>(); 
        for(int i = 0; i<256; i++){ 
          if(temp.existe(i)){ 
             t->agrega(i); 
          } 
       } 
        return t; 
    } 
}; 
 
template <typename T> 
void agrega_seguro(std::unique_ptr<tabla_interfaz<T> >& p, uint8_t c, const T& v) { 
   if (p->existe(c)) { 
     p->agrega(c,v); 
   } else{ 
      if (p->con_capacidad() == false) { 
         p = p->clona_mayor(); 
      } 
      p->agrega(c,v); 
   } 
 
} 
 
void agrega_seguro(std::unique_ptr<tabla_interfaz<void> >& p, uint8_t c) { 
   if (p->existe(c)) { 
     p->agrega(c); 
   } else{ 
      if (p->con_capacidad() == false) { 
         p = p->clona_mayor(); 
      } 
      p->agrega(c); 
   } 
} 

int  main( ) {
    std::unique_ptr<tabla_interfaz<double>> p = std::make_unique<tabla_polimorfica<double, 0>>( );
    std::cout << p->existe(1) << "\n";
    for(int i = 0; i<500; i++){
        agrega_seguro(p, i%256,3.33 + i * 4);
        std::cout << p->existe(i%256) << " " << *p->consulta(i%256) << "\n";
        std::cout << p->existe(i%255 + 1) << " " << *p->consulta(i%255 + 1) << "\n";
    }


}


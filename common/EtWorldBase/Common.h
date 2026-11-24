#pragma once

// Simless Grid Index
struct SectorIndex {
	int nX;
	int nY;

	SectorIndex() { nX = nY = -1; };
	SectorIndex( int x, int y ) { nX = x; nY = y; };
	bool operator == ( const SectorIndex &p ) const {
		if( p.nX == nX && p.nY == nY ) return true;
		return false;
	}
	bool operator != ( const SectorIndex &p ) const {
		if( p.nX == nX && p.nY == nY ) return false;
		return true;
	}
	bool operator == ( int p ) {
		if( nX == p && nY == p ) return true;
		return false;
	}
	bool operator != ( int p ) {
		if( nX == p && nY == p ) return false;
		return true;
	}
	SectorIndex &operator = ( int p ) {
		nX = nY = p;
		return *this;
	}
};

struct IntVec3 {
	int nX;
	int nY;
	int nZ;

	IntVec3() { nX = nY = nZ = -1; }
	IntVec3( int x, int y, int z ) { nX = x; nY = y; nZ = z; }
	bool operator == ( IntVec3 &p ) {
		if( p.nX == nX && p.nY == nY && p.nZ == nZ ) return true;
		return false;
	}
	bool operator != ( IntVec3 &p ) {
		if( p.nX == nX && p.nY == nY && p.nZ == nZ ) return false;
		return true;
	}
	bool operator += ( IntVec3 &p ) {
		nX += p.nX;
		nY += p.nY;
		nZ += p.nZ;
	}
	bool operator -= ( IntVec3 &p ) {
		nX -= p.nX;
		nY -= p.nY;
		nZ -= p.nZ;
	}
	bool operator == ( int p ) {
		if( p == nX && p == nY && p == nZ ) return true;
		return false;
	}
	bool operator != ( int p ) {
		if( p == nX && p == nY && p == nZ ) return false;
		return true;
	}

	IntVec3 &operator = ( int p ) {
		nX = nY = nZ = p;
		return *this;
	}
};
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <random>

using namespace std;

enum suit_type { SPADES, HEARTS, DIAMONDS, CLUBS };
enum rank_type { TWO = 2, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE };

struct card {
    rank_type rank;
    suit_type suit;

    bool operator==( const card& other ) const {
        return rank == other.rank && suit == other.suit;
    }

    bool operator<( const card& other ) const {
        return rank < other.rank;
    }
};

struct hand {
    uint_fast8_t type;
    vector<rank_type> ranks;

    bool operator<( const hand& other ) const {
        return type < other.type;
    }
};

static inline bool is_straight( const vector<rank_type>& ranks ) {
    for ( uint_fast8_t i( 0 ); i < ranks.size() - 1; ++i )
        if ( ranks[i] != ranks[i + 1] - 1 ) return false;

    return true;
}

static inline bool is_flush( const vector<card>& hand ) {
    suit_type suit( hand[0].suit );
    for ( const auto& card : hand )
        if ( card.suit != suit ) return false;

    return true;
}

hand evaluate_hand( const vector<card>& hand ) {
    // setup
    map<rank_type, int> count;
    for ( const auto& card : hand )
        ++count[card.rank];

    vector<rank_type> ranks;
    for ( const auto& [rank, count] : count )
        for ( uint_fast8_t i( 0 ); i < count; ++i )
            ranks.push_back( rank );

    sort( ranks.begin(), ranks.end() );

    bool flush( is_flush( hand ) );
    bool straight( is_straight( ranks ) );

    // straight flush
    if ( flush && straight ) return { 8, ranks };

    if ( count.size() == 2 ) {
        // four of a kind
        if ( count.begin()->second == 1 || count.begin()->second == 4 ) return { 7, ranks };
        
        // full house
        return { 6, ranks };
    }

    // flush
    if ( flush ) return { 5, ranks };
    
    // straight
    if ( straight ) return { 4, ranks };

    if ( count.size() == 3 ) {
        // three of a kind
        if ( count.begin()->second == 3 || next( count.begin() )->second == 3 ) return { 3, ranks };
        
        // two pair
        return { 2, ranks };
    }
    
    // one pair
    if ( count.size() == 4 ) return { 1, ranks };

    // high card
    return { 0, ranks };
}

uint_fast8_t best_hand( const vector<card>& cards ) {
    // setup
    vector<hand> rankings;
    vector<bool> v( cards.size() );
    fill( v.begin(), v.begin() + 5, true );

    // looping through every permutation of cards
    do {
        vector<card> hand;
        for ( uint_fast8_t i = 0; i < cards.size(); ++i ) {
            if ( v[i] ) hand.push_back( cards[i] );
        }

        rankings.push_back( evaluate_hand( hand ) );
    } while ( prev_permutation( v.begin(), v.end() ) );

    // selecting the best hand from the 7 cards
    sort( rankings.begin(), rankings.end(), []( const hand& a, const hand& b ) {
        return a.type > b.type;
    } );

    return rankings.front().type;
}

void simulate( const vector<card>& cards, uint_fast64_t amount ) {
    random_device dev;
    uniform_int_distribution<int> random_card( 0, 51 );
    uint_fast8_t random;
    uint_fast8_t best;

    // initializing main deck
    vector<card> deck;
    for ( uint_fast8_t suit(SPADES); suit <= CLUBS; ++suit )
        for ( uint_fast8_t rank(TWO); rank <= ACE; ++rank )
            deck.push_back( card{ static_cast<rank_type>(rank), static_cast<suit_type>(suit) } );
    
    vector<uint_fast64_t> wins;
    wins.resize( 9, 0 );
    uint_fast8_t number( cards.size() );

    for ( uint_fast64_t i( 0 ); i < amount; ++i ) {
        // copying deck
        vector<card> current_deck = deck;
        vector<card> current_cards = cards;
        
        // removing the cards from current decks
        current_deck.erase( std::remove_if( current_deck.begin(), current_deck.end(),
            [&cards]( const card& card ) {
                return std::find( cards.begin(), cards.end(), card ) != cards.end();
        } ), current_deck.end() );

        // selecting random cards
        for ( uint_fast8_t c( 0 ); c < (7 - cards.size()); ++c ) {
            // randomizing
            random = random_card( dev ) % current_deck.size();

            // pushing the selected card
            current_cards.push_back( current_deck[random] );

            // removing it from the deck
            current_deck.erase( current_deck.begin() + random );
        }

        // calculating the hand
        best = best_hand( current_cards );

        // incrementing the wins
        ++wins[best];
    }

    // results
    cout << " Chances of a given set >> \n";
    for ( uint_fast8_t i( 0 ); i < 9; ++i ) {
        cout << int( i + 1 ) << ". " << float( wins[i] ) / float( amount ) * 100 << " % \n";
    }
}

int main() {
    vector<card> cards = {
        {TWO, SPADES}, {ACE,HEARTS},
    };

    simulate( cards, 10000 );
}

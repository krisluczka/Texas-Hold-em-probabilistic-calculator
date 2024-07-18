#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <random>
#include <chrono>
#include <thread>

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

struct item {
    uint_fast64_t amount;
    uint_fast8_t hand;

    item() : amount( 0 ), hand( 0 ) {}
    item( uint_fast64_t a, uint_fast8_t h ) : amount( a ), hand( h ) {}
};

const string hand_name[] = { "High Card", "One Pair", "Two Pair", "Three of a Kind", "Straight", "Flush", "Full House", "Four of a Kind", "Straight Flush" };

static inline void cls() {
#if defined(_WIN32) || defined(_WIN64)
    system( "cls" );
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__unix__) || defined(__sun)
    system( "clear" );
#endif
}

void sleep( uint_fast64_t milliseconds ) {
    std::this_thread::sleep_for( std::chrono::milliseconds( milliseconds ) );
}

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

void simulate( const vector<card>& deck, const vector<card>& cards, uint_fast64_t amount ) {
    random_device dev;
    uniform_int_distribution<int> random_card( 0, 2147483647 );
    uint_fast8_t random( 0 );
    uint_fast8_t best( 0 );

    // setup
    vector<item> wins;
    wins.reserve( 9 );
    for ( uint_fast8_t i( 0 ); i < 9; ++i )
        wins.push_back( item( 0, i ) );

    for ( uint_fast64_t i( 0 ); i < amount; ++i ) {
        // copying deck
        vector<card> current_deck( deck );
        vector<card> current_cards( cards );
        
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
        ++wins[best].amount;
    }

    // descending sort
    sort( wins.begin(), wins.end(), []( const item& a, const item& b ) {
        return a.amount > b.amount;
    });

    // results
    cout << "\n Chances of a given set >> \n";
    for ( uint_fast8_t i( 0 ); i < 9; ++i ) {
        cout << "     " << int(9 - wins[i].hand) << ". " << hand_name[wins[i].hand] << " - " << float(wins[i].amount) / float(amount) * 100 << " % \n";
    }
}

static inline rank_type string_to_rank( const string& card ) {
    switch ( card[0] ) {
        case '2': return TWO;
        case '3': return THREE;
        case '4': return FOUR;
        case '5': return FIVE;
        case '6': return SIX;
        case '7': return SEVEN;
        case '8': return EIGHT;
        case '9': return NINE;
        case 'T': return TEN;
        case 't': return TEN;
        case 'J': return JACK;
        case 'j': return JACK;
        case 'Q': return QUEEN;
        case 'q': return QUEEN;
        case 'K': return KING;
        case 'k': return KING;
        case 'A': return ACE;
        case 'a': return ACE;
    }
    return TWO;
}

static inline suit_type string_to_suit( const string& card ) {
    switch ( card[1] ) {
        case 'S': return SPADES;
        case 's': return SPADES;
        case 'C': return CLUBS;
        case 'c': return CLUBS;
        case 'D': return DIAMONDS;
        case 'd': return DIAMONDS;
        case 'H': return HEARTS;
        case 'h': return HEARTS;
    }
    return SPADES;
}

void game( bool custom = false ) {
    cls();
    string input;
    // initializing main deck
    vector<card> deck;
    if ( custom ) {
        cout << "\n Type cards to insert into a deck\n\n";
        cout << " 2 3 4 5 6 7 8 9 T J Q K A \n";
        cout << " S (spades)   C (clubs)   D (diamonds)   H (hearts) \n";
        cout << " To finish the deck type X \n\n";

        do {
            cout << " >> ";
            cin >> input;
            deck.push_back( { string_to_rank( input ), string_to_suit( input ) } );
        } while ( input[0] != 'X' && input[0] != 'x' );
    } else {
        for ( uint_fast8_t suit( SPADES ); suit <= CLUBS; ++suit )
            for ( uint_fast8_t rank( TWO ); rank <= ACE; ++rank )
                deck.push_back( card{ static_cast<rank_type>(rank), static_cast<suit_type>(suit) } );
    }


    while ( true ) {
        cls();
        vector<card> cards;
        cards.reserve( 7 );

        // two starting cards
        cout << "\n <-----> \n";
        cout << " To start new simulation type X \n\n";
        cout << " 2 3 4 5 6 7 8 9 T J Q K A \n";
        cout << " S (spades)   C (clubs)   D (diamonds)   H (hearts) \n\n";
        for ( uint_fast8_t i( 0 ); i < 2; ++i ) {
            cout << " Hole card >> "; cin >> input;

            if ( input[0] == 'X' || input[0] == 'x' ) return;

            cards.push_back( { string_to_rank( input ), string_to_suit( input ) } );
        }
        simulate( deck, cards, 10000 );
        cout << "\n <-----> \n";

        // three community cards
        cout << " To start new simulation type X \n\n";
        cout << " 2 3 4 5 6 7 8 9 T J Q K A \n";
        cout << " S (spades)   C (clubs)   D (diamonds)   H (hearts) \n\n";
        for ( uint_fast8_t i( 0 ); i < 3; ++i ) {
            cout << " Community card >> "; cin >> input;

            if ( input[0] == 'X' || input[0] == 'x' ) return;

            cards.push_back( { string_to_rank( input ), string_to_suit( input ) } );
        }
        simulate( deck, cards, 10000 );
        cout << "\n <-----> \n";

        // two community cards
        for ( uint_fast8_t i( 0 ); i < 2; ++i ) {
            cout << " To start new simulation type X \n\n";
            cout << " 2 3 4 5 6 7 8 9 T J Q K A \n";
            cout << " S (spades)   C (clubs)   D (diamonds)   H (hearts) \n\n";
            cout << " Community card >> "; cin >> input;

            if ( input[0] == 'X' || input[0] == 'x' ) return;

            cards.push_back( { string_to_rank( input ), string_to_suit( input ) } );
            simulate( deck, cards, 10000 );
            cout << "\n <-----> \n";
        }

        cin.ignore();
        cin.get();
    }
}

int main() {
    string input;
    cout << " /--------------------------------------\\ \n";
    cout << " |                                      |\n";
    cout << " |   Texas Hold'em probabilistic        |\n";
    cout << " |               calculator version 2   |\n";
    cout << " |                                      |\n";
    cout << " |                by Krzysztof Luczka   |\n";
    cout << " |                                      |\n";
    cout << " \\--------------------------------------/ \n\n";

    cout << "     Type every card that was in the game \n"
        << " (first player's, then the drawn ones). \n"
        << " The calculator will then simulate through \n"
        << " thousands of games to reveal the actual\n"
        << " chances of getting a given hand.\n\n\n";

    //sleep( 3000 );
    cout << " 1. Standard deck\n";
    cout << " 2. Custom deck\n\n";
    cout << " >> "; cin >> input;

    if ( input == "2" )
        game( true );
    else
        game( false );


    
}

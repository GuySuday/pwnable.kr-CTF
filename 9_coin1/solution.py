import re

import pwn

REGULAR_COIN_WEIGHT = 10
NUMBER_OF_GAMES_TO_WIN = 100

def binary_search_coin(connection: pwn.remote, n: int) -> int:
    left, right = 0, n
    while left <= right:
        if left == right:
            return left
        mid = (left + right) // 2

        # Send the server a request to weight all the numbers in range [left, mid]
        weight_request = ' '.join(str(x) for x in range(left, mid + 1)).encode('utf-8')
        connection.sendline(weight_request)

        # Read the weight the server sent
        weight_response = connection.recvline()
        weight = int(weight_response)

        # Expected weight is number of numbers times the weight of each regular coin
        regular_coins_weight = (mid - left + 1) * REGULAR_COIN_WEIGHT

        if weight != regular_coins_weight:
            # Special coin is on the left
            right = mid
        else:
            # Special coin is on the right
            left = mid + 1

    raise RuntimeError("Not possible! Didn't find a special coin")


def win_game(connection: pwn.remote) -> None:
    # Parsing the configuration line of format "N=<n> C=<c>"
    match = connection.recvregex(rb"N=(?P<N>\d+) C=(?P<C>\d+)\n", capture=True)
    if match is None:
        print("Couldn't find configuration line...")
        return
    configuration = match.groupdict()

    n = int(configuration.get('N', 0))
    c = int(configuration.get('C', 0))


    # Find the special coin
    special_coin = binary_search_coin(connection, n)

    special_coin_request = str(special_coin).encode('utf-8')

    while True:
        connection.sendline(special_coin_request)
        special_coin_response = connection.recvline()

        match = re.match(rb"Correct! \((?P<attempt>\d+)\)", special_coin_response)
        if match is not None:
            break

def main() -> None:
    connection = pwn.remote('localhost', 9007)

    for _ in range(0, NUMBER_OF_GAMES_TO_WIN):
        win_game(connection)

    congrats_response = connection.recvline()
    print(congrats_response.decode('utf-8'))
    flag_response = connection.recvline()
    print(flag_response.decode('utf-8'))
   


if "__main__" == __name__:
    main()

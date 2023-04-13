
import json
from typing import List, Dict

#splits raw message from socket into 1 or more messages
# prevents multi back-to-back messages
def split_json(s: str) -> List[Dict]:
    begin = 0
    end = 0
    counter = 0

    l = []
    while end < len(s):
        if s[end] == "{":
            counter += 1
        elif s[end] == "}":
            counter -= 1

            if counter == 0:
                # print(s[begin:end+1])
                l.append(s[begin:end + 1])
                end += 1
                begin = end
                continue
        end += 1

    json_list = []

    for string in l:
        try:
            j = json.loads(string)
            json_list.append(j)
        except Exception as e:
            print(e)

    return json_list

if __name__ == "__main__":
    s = "{bruh{}{}{}}{{yeet}}"
    l = split_json(s)
    for i in l:
        print(i)

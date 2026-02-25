import argparse
from collections import defaultdict
import random
import re


def main(args: argparse.Namespace) -> None:
    with open(args.dataset_path) as f:
        content = f.read()
        content = [
            word.lower() for word in re.sub(r"[^a-zA-Z0-9\s]", "", content).split()
        ]

    words = set(content)

    transition: dict[str, dict[str, int]] = defaultdict(lambda: defaultdict(int))
    for i in range(len(content) - 1):
        transition[content[i]][content[i + 1]] += 1

    last_word: str = ""
    if args.start_text:
        print(args.start_text, end="")
        last_word = args.start_text.split()[-1].lower()

    if last_word not in words:
        last_word = random.choice(list(words))

    if not args.start_text:
        print(last_word, end="")

    for _ in range(args.length):
        print(" ", end="")
        next_words = transition[last_word]
        if not next_words:
            last_word = random.choice(list(words))
        else:
            total = sum(next_words.values())
            last_word = random.choices(
                list(next_words.keys()),
                weights=[c / total for c in next_words.values()],
            )[0]
        print(last_word, end="")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Simple Markov Chain based text generator"
    )
    _ = parser.add_argument(
        "--dataset_path",
        type=str,
        help="Path to dataset (default: alice_in_wonderland)",
        default="../utils/datasets/alice_in_wonderland.txt",
    )
    _ = parser.add_argument(
        "--start_text", type=str, help="Starting text (default: '')", default=""
    )
    _ = parser.add_argument(
        "--length", type=int, help="Length of output string (default: 10)", default=10
    )
    args = parser.parse_args()
    main(args)

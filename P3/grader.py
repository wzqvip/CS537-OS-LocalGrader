import re

def parse_test_scores(file_path):
    # Pattern to match "TEST SCORE: x / y" format
    score_pattern = re.compile(r"TEST SCORE: (\d+) / (\d+)")
    section_pattern = re.compile(r"Running (\w+) tests...")

    total_score = 0
    total_possible = 0
    current_section = ""

    with open(file_path, "r") as file:
        for line in file:
            # Check for new section
            section_match = section_pattern.match(line)
            if section_match:
                current_section = section_match.group(1)

            # Check for score line
            match = score_pattern.search(line)
            if match:
                # Add found scores to totals
                score = int(match.group(1))
                possible = int(match.group(2))
                total_score += score
                total_possible += possible

                # Print current section score
                print(f"{current_section.capitalize()} Test Score: {score} / {possible}")

    # Return total score and total possible score
    return total_score, total_possible

# Replace with the actual path to your runtests output file
file_path = "test_results.txt"
total_score, total_possible = parse_test_scores(file_path)
print(f"Total Score: {total_score} / {total_possible}")

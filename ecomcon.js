const rx_crlf = /\n|\r\n?/;
const rx_ecomcon = /^\/\/([a-zA-Z0-9_]+)\u0020?(.*)$/;
const rx_tag = /^[a-zA-Z0-9_]+$/;

export default function (source_string, tag_array, comment_array = []) {
    const tag = Object.create(null);
    tag_array.forEach(
        function (string) {
            if (!rx_tag.test(string)) {
                throw new Error("ecomcon: " + string);
            }
            tag[string] = true;
        }
    );
    return comment_array.map(
        function (comment) {
            return "// " + comment + "\n";
        }
    ).concat(
        source_string.split(rx_crlf).map(function (line) {
            const array = line.match(rx_ecomcon);
            return (!Array.isArray(array))
                ? line + "\n"
                : (tag[array[1]] === true)
                    ? array[2] + "\n"
                    : "";
        })
    ).join("");
};

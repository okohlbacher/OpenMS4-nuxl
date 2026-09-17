cask "openms4-nuxl" do
  arch arm: "arm64", intel: "x64"

  version "1.0.0-ci.4,e7b88ecf4e40"
  sha256 arm:   "f59b0958ffb75b24b64752a05a145a5dec21ae76bdbe46bb1185d8e3baa3766a",
         intel: "4494e99d165301fae73104478bae5a60995d1efaf2682a2a33353cee20a92166"

  url "https://github.com/okohlbacher/OpenMS4-nuxl/releases/download/" \
      "nuxl-v#{version.csv.first}/OpenMS4-nuxl-macos-#{arch}-Homebrew-#{version.csv.second}.tar.gz"
  name "OpenNuXL"
  desc "Search engine for protein-nucleic acid cross-links, built against the OpenMS Core SDK"
  homepage "https://github.com/okohlbacher/OpenMS4-nuxl"

  depends_on formula: "okohlbacher/openms4-core/openms4-core"
  depends_on macos: :sequoia

  payload = "OpenMS4-nuxl-macos-#{arch}-Homebrew-#{version.csv.second}"
  binary "#{payload}/bin/OpenNuXL"

  # libOpenMS has no versioned name, so a payload only runs with the Core it was built against.
  preflight do
    config = "#{HOMEBREW_PREFIX}/opt/openms4-core/lib/cmake/OpenMS/OpenMSConfig.cmake"
    core = File.exist?(config) ? File.read(config)[/set\(OpenMS_SOURCE_REVISION "([0-9a-f]{40})"\)/, 1] : nil
    next if core == "84847138c0de67149601aaa860af7ac8e2e64534"

    raise Cask::CaskError, "openms4-nuxl #{version.csv.first} was built against openms4-core 84847138c0de, " \
                           "but the installed openms4-core is #{core&.slice(0, 12) || "unknown"}. " \
                           "Install the openms4-nuxl release built for the installed Core."
  end

  postflight_steps do
    run "/usr/bin/xattr",
        args:           ["-dr", "com.apple.quarantine", "."],
        chdir:          ".",
        writable_paths: ["."]
  end
end

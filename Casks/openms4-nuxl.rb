cask "openms4-nuxl" do
  arch arm: "arm64", intel: "x64"

  version "1.0.0-ci.8,2cb38be1e374"
  sha256 arm:   "70615b5a52696fd331875da27ef545135e8b068262067f3857fe424a3652d4eb",
         intel: "af1d9bc576f6f1f27b6c19f64e641ad21d01bc190e7f942c9c6b89657fea4cb1"

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
    next if core == "0529ec8bfe0785d546dab56f58456525ef0fef02"

    raise Cask::CaskError, "openms4-nuxl #{version.csv.first} was built against openms4-core 0529ec8bfe07, " \
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

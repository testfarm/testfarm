<?xml version="1.0"?>

<!-- TestFarm Report Layout Configuration

This file is part of TestFarm,
the Test Automation Tool for Embedded Software.
Please visit http://www.testfarm.org.

TestFarm is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TestFarm is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
-->

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:param name="report-config" select="''"/>

<xsl:variable name="report-config-file">
  <xsl:choose>
    <xsl:when test="document($report-config)/REPORT_CONFIG">
      <xsl:value-of select="$report-config"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="'/opt/testfarm/lib/report.xml'"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:variable>

<xsl:variable name="show_in_title" select="document($report-config-file)//show_in_title != 0"/>
<xsl:variable name="show_in_header" select="document($report-config-file)//show_in_header != 0"/>
<xsl:variable name="show_in_verdict" select="document($report-config-file)//show_in_verdict != 0"/>
<xsl:variable name="show_operator" select="document($report-config-file)//show_operator != 0"/>
<xsl:variable name="show_duration" select="document($report-config-file)//show_duration != 0"/>
<xsl:variable name="show_scenario" select="document($report-config-file)//show_scenario != 0"/>
<xsl:variable name="show_case" select="document($report-config-file)//show_case != 0"/>
<xsl:variable name="show_log" select="document($report-config-file)//show_log != 0"/>
<xsl:variable name="show_validation_parens" select="document($report-config-file)//show_validation_parens != 0"/>
<xsl:variable name="show_validation_state" select="document($report-config-file)//show_validation_state != 0"/>
<xsl:variable name="show_criticity" select="document($report-config-file)//show_criticity != 0"/>
<xsl:variable name="show_nonsignificant" select="document($report-config-file)//show_nonsignificant != 0"/>
<xsl:variable name="show_verdict_passed" select="document($report-config-file)//show_verdict_passed != 0"/>
<xsl:variable name="show_verdict_failed" select="document($report-config-file)//show_verdict_failed != 0"/>
<xsl:variable name="show_verdict_inconclusive" select="document($report-config-file)//show_verdict_inconclusive != 0"/>
<xsl:variable name="show_verdict_skip" select="document($report-config-file)//show_verdict_skip != 0"/>
<xsl:variable name="show_dump" select="document($report-config-file)//show_dump != 0"/>
<xsl:variable name="show_dump_passed" select="document($report-config-file)//show_dump_passed != 0"/>
<xsl:variable name="show_dump_failed" select="document($report-config-file)//show_dump_failed != 0"/>
<xsl:variable name="show_dump_inconclusive" select="document($report-config-file)//show_dump_inconclusive != 0"/>
<xsl:variable name="show_dump_skip" select="document($report-config-file)//show_dump_skip != 0"/>

<xsl:template name="report-config-show">
  <br/>Report Config Parameter: <xsl:value-of select="$report-config"/>
  <br/>Report Config File: <xsl:value-of select="$report-config-file"/>
  <br/>Show IN_TITLE: <xsl:value-of select="$show_in_title"/>
  <br/>Show IN_HEADER: <xsl:value-of select="$show_in_header"/>
  <br/>Show IN_VERDICT: <xsl:value-of select="$show_in_verdict"/>
  <br/>Show Operator: <xsl:value-of select="$show_operator"/>
  <br/>Show Duration: <xsl:value-of select="$show_duration"/>
  <br/>Show Scenario: <xsl:value-of select="$show_scenario"/>
  <br/>Show Test Cases: <xsl:value-of select="$show_case"/>
  <br/>Show HTML Log: <xsl:value-of select="$show_log"/>
  <br/>Show Names in Parenthesis if not Approved: <xsl:value-of select="$show_validation_parens"/>
  <br/>Show Validation State: <xsl:value-of select="$show_validation_state"/>
  <br/>Show Criticity: <xsl:value-of select="$show_criticity"/>
  <br/>Show Non-significant Test Cases: <xsl:value-of select="$show_nonsignificant"/>
  <br/>Show Verdict PASSED: <xsl:value-of select="$show_verdict_passed"/>
  <br/>Show Verdict FAILED: <xsl:value-of select="$show_verdict_failed"/>
  <br/>Show Verdict INCONCLUSIVE: <xsl:value-of select="$show_verdict_inconclusive"/>
  <br/>Show Verdict SKIP: <xsl:value-of select="$show_verdict_skip"/>
  <br/>Show Output Dump: <xsl:value-of select="$show_dump"/>
  <br/>Show Output Dump (PASSED): <xsl:value-of select="$show_dump_passed"/>
  <br/>Show Output Dump (FAILED): <xsl:value-of select="$show_dump_failed"/>
  <br/>Show Output Dump (INCONCLUSIVE): <xsl:value-of select="$show_dump_inconclusive"/>
  <br/>Show Output Dump (SKIP): <xsl:value-of select="$show_dump_skip"/>
</xsl:template>

</xsl:stylesheet>

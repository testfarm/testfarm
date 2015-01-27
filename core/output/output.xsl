<?xml version="1.0"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<!-- TestFarm Output Display Format -->
<!-- $Revision: 371 $ -->
<!-- $Date: 2007-02-26 18:58:01 +0100 (lun., 26 févr. 2007) $ -->

<!-- Test Cases to Scenario results aggregation rules:                                          -->
<!-- * Verdict :                                                                                -->
<!--   - PASSED if sequence contains at least one PASSED and no FAILED/INCONCLUSIVE;            -->
<!--   - FAILED if sequence contains at least one FAILED test case;                             -->
<!--   - INCONCLUSIVE if sequence contains at least one INCONCLUSIVE and no FAILED              -->
<!--   - SKIP if sequence contains SKIP test cases only                                         -->
<!-- * Criticity:                                                                               -->
<!--   If verdict is X, take the highest criticity of the test cases that produced this verdict -->
<!-- * Validated:                                                                               -->
<!--   If all test cases are validated                                                          -->

<xsl:output method="html"/>

<xsl:param name="signature" select="''"/>

<xsl:variable name="color-table-header" select="'#C0C0C0'"/>
<xsl:variable name="color-table-odd" select="'#F0F0F0'"/>
<xsl:variable name="color-table-even" select="'#E0E0E0'"/>

<xsl:include href="/opt/testfarm/lib/styles.xsl"/>
<xsl:include href="/opt/testfarm/lib/report.xsl"/>
<xsl:include href="/opt/testfarm/lib/validate.xsl"/>
<xsl:include href="/opt/testfarm/lib/verdict.xsl"/>
<xsl:include href="/opt/testfarm/lib/criticity.xsl"/>
<xsl:include href="/opt/testfarm/lib/title.xsl"/>
<xsl:include href="/opt/testfarm/lib/text.xsl"/>


<xsl:template name="verdict-preamble">
  <xsl:param name="which"/>

  <xsl:if test="$show_criticity">
    <i><b><u>C</u></b>riticity = { </i>
    <xsl:call-template name="criticity-summary"/>
    <i> }</i><br/>
  </xsl:if>

  <i><b><u>V</u></b>erdict = { </i>
  <xsl:call-template name="verdict-summary"/>
  <i> }</i><br/>

  <i>Names in parenthesis denote <xsl:value-of select="$which"/> that are not
  <xsl:call-template name="validate-display">
    <xsl:with-param name="level" select="$validate-max"/>
  </xsl:call-template>.</i><br/>

  <xsl:if test="not($show_nonsignificant)">
    <i>Only significant <xsl:value-of select="$which"/> (i.e. with a criticity) are shown.</i><br/>
  </xsl:if>
</xsl:template>


<xsl:template match="OUTPUT">
  <xsl:variable name="tree-name" select="@id"/>

  <html>

    <!-- ================== -->
    <!-- = DOCUMENT TITLE = -->
    <!-- ================== -->
    <head>
      <title>TestFarm Report: <xsl:value-of select="@id"/></title>
      <xsl:call-template name="styles" />
      <xsl:if test="$signature!=''">
        <meta name="signature" content="{$signature}"/>
      </xsl:if>
    </head>

    <body>
      <xsl:call-template name="title"/>


      <!-- =================================================== -->
      <!-- = TITLE INFORMATION                               = -->
      <!-- = From XML output file and IN_TITLE stdio output  = -->
      <!-- =================================================== -->
      <div class="in_title">
        <xsl:if test="$show_in_title">
          <xsl:for-each select=".//STDIO[@channel = 'in_title']">
            <xsl:call-template name="text-display">
              <xsl:with-param name="string" select="."/>
              <xsl:with-param name="filter" select="'IN_TITLE'"/>
            </xsl:call-template>
          </xsl:for-each>
        </xsl:if>

        Test suite name: <b><xsl:value-of select="@id"/></b>
        <br/>

        <xsl:if test="DESCRIPTION">
          Description: <b><xsl:value-of select="DESCRIPTION"/></b>
          <br/>
        </xsl:if>

        <xsl:if test="REFERENCE">
          Test specification: <b><xsl:value-of select="REFERENCE"/></b>
          <br/>
        </xsl:if>

        <xsl:if test="OPERATOR">
          Operator's name: <b><xsl:value-of select="OPERATOR"/></b>
          <br/>
        </xsl:if>

        <xsl:if test="RELEASE">
          Release: <b><xsl:value-of select="RELEASE"/></b>
          <br/>
        </xsl:if>
      </div>


      <!-- =============================== -->
      <!-- = GLOBAL INFORMATION          = -->
      <!-- = From IN_HEADER stdio output = -->
      <!-- =============================== -->
      <xsl:if test="$show_in_header">
        <h2>Global Information</h2>
        <div class="in_header">
          <xsl:for-each select=".//STDIO[@channel='in_header']">
            <xsl:call-template name="text-display">
              <xsl:with-param name="string" select="."/>
              <xsl:with-param name="filter" select="'IN_HEADER'"/>
            </xsl:call-template>
          </xsl:for-each>
        </div>
      </xsl:if>


      <!-- =================== -->
      <!-- = VERDICT SUMMARY = -->
      <!-- =================== -->
      <h2>Verdict summary</h2>
      <div class="summary">
        <xsl:variable name="finish-date" select="descendant::VERDICT[last()]/@date"/>
        Start date: <b><xsl:value-of select="@localtime"/></b><br/>
        Finish date: <b><xsl:value-of select="descendant::VERDICT[last()]/@localtime"/></b><br/>
        <xsl:if test="$show_duration">
          Duration: <b>
          <xsl:call-template name="duration-display">
            <xsl:with-param name="duration" select="$finish-date - @date"/>
          </xsl:call-template>
          </b><br/>
        </xsl:if>
      </div>
      <br/>

      <xsl:variable name="nscenario-counted" select="count(.//SEQ[@scenario])"/>
      <xsl:variable name="nscenario" select="number($show_scenario) * $nscenario-counted"/>
      <xsl:if test="not($nscenario-counted)">
        <i>No scenario information found</i><br/><br/>
      </xsl:if>
      <xsl:variable name="nscenario-passed" select="count(.//SEQ[@scenario and count(CASE[@criticity and (VERDICT = $verdict-passed)]) and not(count(CASE[@criticity and ((VERDICT = $verdict-failed) or (VERDICT = $verdict-inconclusive))]))])"/>
      <xsl:variable name="nscenario-failed" select="count(.//SEQ[@scenario and count(CASE[@criticity and (VERDICT = $verdict-failed)])])"/>
      <xsl:variable name="nscenario-inconclusive" select="count(.//SEQ[@scenario and count(CASE[@criticity and (VERDICT = $verdict-inconclusive)]) and not(count(CASE[@criticity and (VERDICT = $verdict-failed)]))])"/>
      <xsl:variable name="nscenario-skip" select="count(.//SEQ[@scenario and (count(CASE[VERDICT = $verdict-skip]) = count(CASE))])"/>
      <xsl:variable name="nscenario-executed" select="$nscenario-counted - $nscenario-skip"/>
      <xsl:variable name="nscenario-significant" select="$nscenario-passed + $nscenario-failed + $nscenario-inconclusive"/>

      <xsl:variable name="pscenario-passed" select="round((100 * $nscenario-passed) div $nscenario-significant)"/>
      <xsl:variable name="pscenario-failed" select="(boolean($nscenario-inconclusive) * round((100 * $nscenario-failed) div $nscenario-significant)) + (not($nscenario-inconclusive) * (100 - $pscenario-passed))"/>
      <xsl:variable name="pscenario-inconclusive" select="100 - $pscenario-passed - $pscenario-failed"/>

      <xsl:variable name="ncase-passed" select="count(.//CASE[@criticity and (VERDICT = $verdict-passed)])"/>
      <xsl:variable name="ncase-failed" select="count(.//CASE[@criticity and (VERDICT = $verdict-failed)])"/>
      <xsl:variable name="ncase-inconclusive" select="count(.//CASE[@criticity and (VERDICT = $verdict-inconclusive)])"/>
      <xsl:variable name="ncase-skip" select="count(.//CASE[VERDICT = $verdict-skip])"/>
      <xsl:variable name="ncase-executed" select="count(.//CASE) - $ncase-skip"/>
      <xsl:variable name="ncase-significant" select="$ncase-passed + $ncase-failed + $ncase-inconclusive"/>

      <xsl:variable name="pcase-passed" select="round((100 * $ncase-passed) div $ncase-significant)"/>
      <xsl:variable name="pcase-failed" select="(boolean($ncase-inconclusive) * round((100 * $ncase-failed) div $ncase-significant)) + (not($ncase-inconclusive) * (100 - $pcase-passed))"/>
      <xsl:variable name="pcase-inconclusive" select="100 - $pcase-passed - $pcase-failed"/>


      <!-- ====================================== -->
      <!-- = VERDICT SUMMARY: Global Statistics = -->
      <!-- ====================================== -->
      <table class="verdict">
        <tr class="header">
          <td class="header" rowspan="2"></td>
          <xsl:if test="$nscenario">
            <td class="header" colspan="2">Scenarios</td>
          </xsl:if>
          <xsl:if test="$show_case">
            <td class="header" colspan="2">Test Cases</td>
          </xsl:if>
        </tr>
        <tr class="header">
          <xsl:if test="$nscenario">
            <td class="header">Nb</td>
            <td class="header">% Tot.</td>
          </xsl:if>
          <xsl:if test="$show_case">
            <td class="header">Nb</td>
            <td class="header">% Tot.</td>
          </xsl:if>
        </tr>
        <tr class="line0">
          <td class="line0" align="right">Total</td>
          <xsl:if test="$nscenario">
            <td class="line0" align="right"><xsl:value-of select="@nscenario"/></td>
            <td class="line0" align="center">-</td>
          </xsl:if>
          <xsl:if test="$show_case">
            <td class="line0" align="right"><xsl:value-of select="@ncase"/></td>
            <td class="line0" align="center">-</td>
          </xsl:if>
        </tr>
        <tr class="line0">
          <td class="line0" align="right">Executed</td>
          <xsl:if test="$nscenario">
            <td class="line0" align="right"><xsl:value-of select="$nscenario-executed"/></td>
            <td class="line0" align="right"><xsl:value-of select="format-number($nscenario-executed div @nscenario, '#%')"/></td>
          </xsl:if>
          <xsl:if test="$show_case">
            <td class="line0" align="right"><xsl:value-of select="$ncase-executed"/></td>
            <td class="line0" align="right"><xsl:value-of select="format-number($ncase-executed div @ncase, '#%')"/></td>
          </xsl:if>
        </tr>
        <tr class="line0">
          <td class="line0" align="right">Significant</td>
          <xsl:if test="$nscenario">
            <td class="line0" align="right"><xsl:value-of select="$nscenario-significant"/></td>
            <td class="line0" align="right"><xsl:value-of select="format-number($nscenario-significant div @nscenario, '#%')"/></td>
          </xsl:if>
          <xsl:if test="$show_case">
            <td class="line0" align="right"><xsl:value-of select="$ncase-significant"/></td>
            <td class="line0" align="right"><xsl:value-of select="format-number($ncase-significant div @ncase, '#%')"/></td>
          </xsl:if>
        </tr>
      </table><br/>


      <!-- ========================================== -->
      <!-- = VERDICT SUMMARY: Statistics by Verdict = -->
      <!-- ========================================== -->
      <table class="verdict">
        <tr class="header">
          <td class="header" colspan="2"></td>
          <xsl:if test="$nscenario">
            <td class="header" colspan="2">Scenarios</td>
          </xsl:if>
          <xsl:if test="$show_case">
            <td class="header" colspan="2">Test Cases</td>
          </xsl:if>
        </tr>
        <tr class="header">
          <td class="header">Verdict</td>
          <td class="header">Meaning</td>
          <xsl:if test="$nscenario">
            <td class="header">Nb</td>
            <td class="header">% Sig.</td>
          </xsl:if>
          <xsl:if test="$show_case">
            <td class="header">Nb</td>
            <td class="header">% Sig.</td>
          </xsl:if>
        </tr>
        <tr class="line0">
          <td class="line0"><b>
            <xsl:call-template name="verdict-display">
              <xsl:with-param name="verdict" select="$verdict-passed"/>
            </xsl:call-template>
          </b></td>
          <td class="line0"><i>The product behaves correctly</i></td>
          <xsl:if test="$nscenario">
            <td class="line0" align="right"><xsl:value-of select="$nscenario-passed"/></td>
            <td class="line0" align="right"><xsl:value-of select="$pscenario-passed"/>%</td>
          </xsl:if>
          <xsl:if test="$show_case">
            <td class="line0" align="right"><xsl:value-of select="$ncase-passed"/></td>
            <td class="line0" align="right"><xsl:value-of select="$pcase-passed"/>%</td>
          </xsl:if>
        </tr>
        <tr class="line0">
          <td class="line0"><b>
            <xsl:call-template name="verdict-display">
              <xsl:with-param name="verdict" select="$verdict-failed"/>
            </xsl:call-template>
          </b></td>
          <td class="line0"><i>A failure was detected in the product</i></td>
          <xsl:if test="$nscenario">
            <td class="line0" align="right"><xsl:value-of select="$nscenario-failed"/></td>
            <td class="line0" align="right"><xsl:value-of select="$pscenario-failed"/>%</td>
          </xsl:if>
          <xsl:if test="$show_case">
            <td class="line0" align="right"><xsl:value-of select="$ncase-failed"/></td>
            <td class="line0" align="right"><xsl:value-of select="$pcase-failed"/>%</td>
          </xsl:if>
        </tr>
        <tr class="line0">
          <td class="line0"><b>
            <xsl:call-template name="verdict-display">
              <xsl:with-param name="verdict" select="$verdict-inconclusive"/>
            </xsl:call-template>
          </b></td>
          <td class="line0"><i>The product/system does not behave as expected: no verdict produced</i></td>
          <xsl:if test="$nscenario">
            <td class="line0" align="right"><xsl:value-of select="$nscenario-inconclusive"/></td>
            <td class="line0" align="right"><xsl:value-of select="$pscenario-inconclusive"/>%</td>
          </xsl:if>
          <xsl:if test="$show_case">
            <td class="line0" align="right"><xsl:value-of select="$ncase-inconclusive"/></td>
            <td class="line0" align="right"><xsl:value-of select="$pcase-inconclusive"/>%</td>
          </xsl:if>
        </tr>
        <tr class="line0">
          <td class="line0"><b>
            <xsl:call-template name="verdict-display">
              <xsl:with-param name="verdict" select="$verdict-skip"/>
            </xsl:call-template>
          </b></td>
          <td class="line0"><i>The scenario or test case is not relevant: it was not executed</i></td>
          <xsl:if test="$nscenario">
            <td class="line0" align="right"><xsl:value-of select="$nscenario-skip"/></td>
            <td class="line0" align="center">-</td>
          </xsl:if>
          <xsl:if test="$show_case">
            <td class="line0" align="right"><xsl:value-of select="$ncase-skip"/></td>
            <td class="line0" align="center">-</td>
          </xsl:if>
       </tr>
     </table>


      <!-- ============================ -->
      <!-- = VERDICT LIST BY SCENARIO = -->
      <!-- ============================ -->
      <xsl:if test="$show_scenario">
        <h2>Verdict list by Scenario</h2>

        <xsl:call-template name="verdict-preamble">
          <xsl:with-param name="which" select="'Scenarios'"/>
        </xsl:call-template>
        <br/>

        <table class="verdict">
          <tr class="header">
            <td class="header">Scenario</td>
            <td class="header">Cases</td>
            <xsl:if test="$show_criticity">
              <td class="header">C.</td>
            </xsl:if>
            <td class="header">Description</td>
            <xsl:if test="$show_duration">
              <td class="header">Duration</td>
            </xsl:if>
            <td class="header">V.</td>
          </tr>
          <xsl:for-each select=".//SEQ[@scenario]">
            <xsl:variable name="first-case" select="CASE[1]/@id"/>
            <xsl:variable name="ncase" select="count(CASE)"/>
            <xsl:variable name="nvalidated" select="count(CASE[number(VALIDATED) = $validate-max])"/>
            <xsl:variable name="class">
              line<xsl:value-of select="position() mod 2"/>
            </xsl:variable>

            <!-- Compute verdict: the highest verdict among significant test cases -->
            <xsl:variable name="passed" select="count(CASE[@criticity and (VERDICT = $verdict-passed)])"/>
            <xsl:variable name="failed" select="count(CASE[@criticity and (VERDICT = $verdict-failed)])"/>
            <xsl:variable name="inconclusive" select="count(CASE[@criticity and (VERDICT = $verdict-inconclusive)])"/>
            <xsl:variable name="skip" select="count(CASE[VERDICT = $verdict-skip])"/>

            <xsl:variable name="verdict0" select="1 + (($failed>0)*8) + (($inconclusive>0)*4) + (($passed>0)*2) + ($skip>0)"/>
            <xsl:variable name="verdict" select="substring('0411333322222222',$verdict0,1) - 1"/>

            <!-- Compute criticity: the highest criticity for the verdict -->
            <xsl:variable name="criticity1" select="boolean(count(CASE[(VERDICT = $verdict) and (number(VERDICT/@criticity) = '1')]))"/>
            <xsl:variable name="criticity2" select="boolean(count(CASE[(VERDICT = $verdict) and (number(VERDICT/@criticity) = '2')]))"/>
            <xsl:variable name="criticity3" select="boolean(count(CASE[(VERDICT = $verdict) and (number(VERDICT/@criticity) = '3')]))"/>
            <xsl:variable name="criticity4" select="boolean(count(CASE[(VERDICT = $verdict) and (number(VERDICT/@criticity) = '4')]))"/>
            <xsl:variable name="criticity5" select="boolean(count(CASE[(VERDICT = $verdict) and (number(VERDICT/@criticity) = '5')]))"/>

            <xsl:variable name="criticity0" select="1 + ($criticity5 * 16) + ($criticity4 * 8) + ($criticity3 * 4) + ($criticity2 * 2) + $criticity1"/>
            <xsl:variable name="criticity" select="substring('01223333444444445555555555555555',$criticity0,1)"/>

            <xsl:if test="(($criticity > 0) or $show_nonsignificant) and ((($verdict=$verdict-passed) and $show_verdict_passed) or (($verdict=$verdict-failed) and $show_verdict_failed) or (($verdict=$verdict-inconclusive) and $show_verdict_inconclusive) or (($verdict=$verdict-skip) and $show_verdict_skip))">
              <tr class="{$class}">
                <td class="{$class}"><a href="#CASE-{$first-case}">
                  <xsl:variable name="not-validated" select="$show_validation_parens and boolean($ncase - $nvalidated)"/>
                  <xsl:if test="$not-validated">(</xsl:if>
                  <xsl:value-of select="@id"/>
                  <xsl:if test="$not-validated">)</xsl:if>
                </a></td>
                <td class="{$class}" align="right">
                  <xsl:value-of select="$ncase"/>
                </td>
                <xsl:if test="$show_criticity">
                  <td class="{$class}" align="center">
                    <xsl:choose>
                      <xsl:when test="$verdict >= 0">
                        <xsl:call-template name="criticity-symbol">
                          <xsl:with-param name="level" select="$criticity"/>
                        </xsl:call-template>
                      </xsl:when>
                      <xsl:otherwise>
                        -
                      </xsl:otherwise>
                    </xsl:choose>
                  </td>
                </xsl:if>
                <td class="{$class}">
                  <xsl:value-of select="DESCRIPTION"/>
                </td>
                <xsl:if test="$show_duration">
                  <td class="{$class}">
                    <xsl:call-template name="duration-display">
                      <xsl:with-param name="duration" select="CASE[last()]/VERDICT/@date - CASE[1]/@date"/>
                    </xsl:call-template>
                  </td>
                </xsl:if>
                <td class="{$class}" align="center">
                  <!--
                  P:<xsl:value-of select="$passed"/>
                  F:<xsl:value-of select="$failed"/>
                  I:<xsl:value-of select="$inconclusive"/>
                  S:<xsl:value-of select="$skip"/> =&gt;
                  -->
                  <xsl:call-template name="verdict-symbol">
                    <xsl:with-param name="verdict" select="$verdict"/>
                  </xsl:call-template>
                </td>
              </tr>
            </xsl:if>
          </xsl:for-each>
        </table><br/>
      </xsl:if>


      <!-- ============================= -->
      <!-- = VERDICT LIST BY TEST CASE = -->
      <!-- ============================= -->
      <xsl:if test="$show_case">
        <h2>Verdict list by Test Case</h2>

        <xsl:call-template name="verdict-preamble">
          <xsl:with-param name="which" select="'Test Cases'"/>
        </xsl:call-template>

        <xsl:if test="$show_log">
          <i>Stars following Test Case names are hyperlinks to Test Log sections.</i><br/>
        </xsl:if>

        <br/>

        <table class="verdict">
          <xsl:variable name="colspan" select="number($show_criticity) + number($show_duration) + 2"/>
          <tr class="header">
            <td class="header" rowspan="2">Test Case</td>
            <xsl:if test="$show_criticity">
              <td class="header">C.</td>
            </xsl:if>
            <td class="header">Description</td>
            <xsl:if test="$show_duration">
              <td class="header">Duration</td>
            </xsl:if>
            <td class="header">V.</td>
          </tr>
          <tr class="header">
            <td class="header" colspan="{$colspan}">Information (if any)</td>
          </tr>
          <xsl:for-each select=".//CASE">
            <xsl:variable name="in-verdict" select="$show_in_verdict and (count(STDIO[contains(.,'IN_VERDICT ')]) > 0)"/>
            <xsl:variable name="rowspan" select="number($in-verdict) + 1"/>
            <xsl:variable name="class">
              line<xsl:value-of select="position() mod 2"/>
            </xsl:variable>

            <xsl:if test="(VERDICT/@criticity or $show_nonsignificant) and (((VERDICT=$verdict-passed) and $show_verdict_passed) or ((VERDICT=$verdict-failed) and $show_verdict_failed) or ((VERDICT=$verdict-inconclusive) and $show_verdict_inconclusive) or ((VERDICT=$verdict-skip) and $show_verdict_skip))">
              <tr class="{$class}">
                <td class="{$class}" rowspan="{$rowspan}">
                  <xsl:variable name="not-validated" select="$show_validation_parens and (number(VALIDATED) != $validate-max)"/>
                  <a name="CASE-{@id}"/>
                  <xsl:if test="$show_dump">
                    <a href="#DUMP-{@id}">
                      <xsl:if test="$not-validated">(</xsl:if>
                      <xsl:value-of select="@id"/>
                      <xsl:if test="$not-validated">)</xsl:if>
                    </a>
                  </xsl:if>
                  <xsl:if test="not($show_dump)">
                    <xsl:if test="$not-validated">(</xsl:if>
                    <xsl:value-of select="@id"/>
                    <xsl:if test="$not-validated">)</xsl:if>
                  </xsl:if>
                  <xsl:if test="$show_log">
                    <xsl:text> </xsl:text>
                    <xsl:variable name="href" select="concat('file:', @id, '.html')"/>
                    <a href="{$href}">*</a>
                  </xsl:if>
               </td>
                <xsl:if test="$show_criticity">
                  <td class="{$class}" align="center">
                    <xsl:call-template name="criticity-symbol">
                      <xsl:with-param name="level" select="VERDICT/@criticity"/>
                    </xsl:call-template>
                  </td>
                </xsl:if>
                <td class="{$class}"><i><xsl:value-of select="DESCRIPTION"/></i></td>
                <xsl:if test="$show_duration">
                  <td class="{$class}">
                    <xsl:call-template name="duration-display">
                      <xsl:with-param name="duration" select="VERDICT/@date - @date"/>
                    </xsl:call-template>
                  </td>
                </xsl:if>
                <td class="{$class}" align="center">
                  <xsl:call-template name="verdict-symbol">
                    <xsl:with-param name="verdict" select="VERDICT"/>
                  </xsl:call-template>
                </td>
              </tr>
              <xsl:if test="$in-verdict">
                <tr class="{$class}">
                  <td class="{$class}" colspan="{$colspan}">
                    <xsl:for-each select="STDIO[@channel='in_verdict']">
                      <xsl:call-template name="text-display">
                        <xsl:with-param name="string" select="."/>
                        <xsl:with-param name="filter" select="'IN_VERDICT'"/>
                      </xsl:call-template>
                    </xsl:for-each>
                  </td>
                </tr>
              </xsl:if>
            </xsl:if>
          </xsl:for-each>
        </table><br/>
      </xsl:if>


      <!-- =============== -->
      <!-- = OUTPUT DUMP = -->
      <!-- =============== -->
      <xsl:if test="$show_dump">
        <h2>Output dump</h2>

        <xsl:call-template name="stdio-display" />

        <xsl:for-each select=".//CASE">
          <xsl:if test="((VERDICT=$verdict-passed) and $show_dump_passed) or ((VERDICT=$verdict-failed) and $show_dump_failed) or ((VERDICT=$verdict-inconclusive) and $show_dump_inconclusive) or ((VERDICT=$verdict-skip) and $show_dump_skip)">
            <h3>
              <a name="DUMP-{@id}"/>
              <a href="#CASE-{@id}">
                <xsl:variable name="not-validated" select="$show_validation_parens and (number(VALIDATED) != $validate-max)"/>
                <xsl:if test="$not-validated">(</xsl:if>
                <xsl:value-of select="@id"/>
                <xsl:if test="$not-validated">)</xsl:if>
              </a>
              <xsl:if test="$show_log">
                <xsl:text> </xsl:text>
                <xsl:variable name="href" select="concat('file:', @id, '.html')"/>
                <a href="{$href}">*</a>
              </xsl:if>
              <xsl:text> : </xsl:text>
              <xsl:if test="$show_duration">
                <xsl:call-template name="duration-display">
                  <xsl:with-param name="duration" select="number(VERDICT/@date)-@date"/>
                </xsl:call-template>
                <xsl:text> : </xsl:text>
              </xsl:if>
              <xsl:call-template name="verdict-display">
                <xsl:with-param name="verdict" select="VERDICT"/>
              </xsl:call-template>
            </h3>

            <xsl:if test="$show_validation_state and boolean(VALIDATED)">
              <small><b>
              <xsl:call-template name="validate-display">
                <xsl:with-param name="level" select="VALIDATED"/>
                <xsl:with-param name="info">
                  <xsl:value-of select="VALIDATED/@localtime"/>
                  <xsl:if test="VALIDATED/@operator">
                    by <xsl:value-of select="VALIDATED/@operator"/>
                  </xsl:if>
                </xsl:with-param>
              </xsl:call-template>
              </b></small>
            </xsl:if>

            <xsl:call-template name="stdio-display" />
          </xsl:if>
        </xsl:for-each>
      </xsl:if>

      <!-- =================== -->
      <!-- = End of document = -->
      <!-- =================== -->
    </body>
  </html>
</xsl:template>

<xsl:template match="LOG">
</xsl:template>

</xsl:stylesheet>

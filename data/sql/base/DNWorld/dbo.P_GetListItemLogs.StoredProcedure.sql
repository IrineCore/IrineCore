USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListItemLogs]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-01-23
description : ??? ??? ??? ?????.
result set : 
		LogDate/datetime2, ItemSerial/bigint, ItemID/int, ItemLogCode/tinyint, OwnerCharacterID/bigint, OwnerCharacterIDBefore/bigint, ItemCount/smallint, ItemCountBefore/smallint, ItemLogFKey1/bigint, ItemLogFKey2/bigint, OwnershipStartDate/datetime2(0), IP/varchar, ItemLevel/tinyint, ItemPotential/tinyint, SealCount/tinyint, ItemOption/tinyint, SoulBoundFlag/bit, ItemLevelBefore/tinyint, ItemDurability/smallint, ItemPotentialMoveCount/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListItemLogs]
	  @dt2StartLogDate datetime2(0) --// ?? ?? ?? ??
	, @dt2EndLogDate datetime2(0) --// ?? ?? ?? ??	
	, @inbCharacterID bigint --// ??? ID	
	, @vchItemLogCodes varchar(max) = NULL --// (optional) ??? ???? (','? ???? ???? ??????. 1=??,2=??,3=???,4=??,5=??(?????),6=??(??),7=??(??),8=??(DNA???),9=??(????),10=??? ??(??),11=??,12=??,13=??,14=??,15=?? ??,16=PVP ??,17=?? ??,18=??? ??, 20=??,21=???,22=???,23=??(??),24=??(??),25=??? ??(?? ??),26=?? ?? ???,27=??? ??,28=??? ??(???),29=??? ??(??),30=??? ??(?? ??),31=??? ??(?? ??,33=?? ???? ??,34=?? ???? ??, 35=???(????), 36=???(New), 37=??? ???? ? ??? ??, 38=??? ??, 39=?? ?? ??, 40=??? ?? ??, 41=??? ??, 42=NPC?? ??, 43=??? ??, 44=??? ?? ?? By ???, 45=?? ??? ??, 46=?? ???? ??, 47=??, 48=??, 49=???, 50=??(PVP?????), 51=??(?? ???), 52=??? ??, 53=??(????????), 54=???, 55=??????, 56=?? ??? ??(?? ?? ? ??), 57=?? ??? ??, 58=(DOORS) ?? ??, 59=(DOORS) ?? ??, 60=(DOORS) ?? ??, 61=???? ?? ??? ??, 62=?????(Target), 63=?????(Source), 64=????, 65=???????, 66=???????, 67=??????, 68=???????, 69=????????, 70=?????, 71=?????????, 72=???????, 73=??????, 74=??(??)	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY	
	IF CHARINDEX(',', @vchItemLogCodes) = 0 OR CHARINDEX(',', @vchItemLogCodes) IS NULL
		IF dbo.FN_LogTableSuffix(GETDATE()) = 0 			
			SELECT LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, dbo.FN_IPNumber2IP(IP) AS IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemDurability, ItemPotentialMoveCount
			FROM dbo.ItemLogs0 WITH (NOLOCK)
			WHERE (OwnerCharacterID = @inbCharacterID OR OwnerCharacterIDBefore = @inbCharacterID)
			  AND ItemLogCode = COALESCE(CAST (@vchItemLogCodes AS tinyint), ItemLogCode)
		      AND LogDate >= @dt2StartLogDate AND LogDate <= @dt2EndLogDate
			OPTION (MAXDOP 1);	
		ELSE IF dbo.FN_LogTableSuffix(GETDATE()) = 1 		
			SELECT LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, dbo.FN_IPNumber2IP(IP) AS IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemDurability, ItemPotentialMoveCount
			FROM dbo.ItemLogs1 WITH (NOLOCK)
			WHERE (OwnerCharacterID = @inbCharacterID OR OwnerCharacterIDBefore = @inbCharacterID)
			  AND ItemLogCode = COALESCE(CAST (@vchItemLogCodes AS tinyint), ItemLogCode)
		      AND LogDate >= @dt2StartLogDate AND LogDate <= @dt2EndLogDate
			OPTION (MAXDOP 1);	
		ELSE 
			SELECT LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, dbo.FN_IPNumber2IP(IP) AS IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemDurability, ItemPotentialMoveCount
			FROM dbo.ItemLogs2 WITH (NOLOCK)
			WHERE (OwnerCharacterID = @inbCharacterID OR OwnerCharacterIDBefore = @inbCharacterID)
              AND ItemLogCode = COALESCE(CAST (@vchItemLogCodes AS tinyint), ItemLogCode)
		      AND LogDate >= @dt2StartLogDate AND LogDate <= @dt2EndLogDate
			OPTION (MAXDOP 1);	
	ELSE		
		IF dbo.FN_LogTableSuffix(GETDATE()) = 0 
			WITH LC AS (
				SELECT CAST (string AS tinyint) AS ItemLogCode
				FROM dbo.TF_Split(@vchItemLogCodes,',')
			)
			SELECT LogDate, ItemSerial, ItemID, IL.ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, dbo.FN_IPNumber2IP(IP) AS IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemDurability, ItemPotentialMoveCount
			FROM dbo.ItemLogs0 IL WITH (NOLOCK) INNER JOIN LC ON IL.ItemLogCode = LC.ItemLogCode
			WHERE (IL.OwnerCharacterID = @inbCharacterID OR IL.OwnerCharacterIDBefore = @inbCharacterID)
		      AND IL.LogDate >= @dt2StartLogDate AND IL.LogDate <= @dt2EndLogDate
			OPTION (MAXDOP 1);	
		ELSE IF dbo.FN_LogTableSuffix(GETDATE()) = 1 		
			WITH LC AS (
				SELECT CAST (string AS tinyint) AS ItemLogCode
				FROM dbo.TF_Split(@vchItemLogCodes,',')
			)
			SELECT LogDate, ItemSerial, ItemID, IL.ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, dbo.FN_IPNumber2IP(IP) AS IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemDurability, ItemPotentialMoveCount
			FROM dbo.ItemLogs1 IL WITH (NOLOCK) INNER JOIN LC ON IL.ItemLogCode = LC.ItemLogCode
			WHERE (IL.OwnerCharacterID = @inbCharacterID OR IL.OwnerCharacterIDBefore = @inbCharacterID)
		      AND IL.LogDate >= @dt2StartLogDate AND IL.LogDate <= @dt2EndLogDate
			OPTION (MAXDOP 1);	
		ELSE 
			WITH LC AS (
				SELECT CAST (string AS tinyint) AS ItemLogCode
				FROM dbo.TF_Split(@vchItemLogCodes,',')
			)
			SELECT LogDate, ItemSerial, ItemID, IL.ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, dbo.FN_IPNumber2IP(IP) AS IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemDurability, ItemPotentialMoveCount
			FROM dbo.ItemLogs2 IL WITH (NOLOCK) INNER JOIN LC ON IL.ItemLogCode = LC.ItemLogCode
			WHERE (IL.OwnerCharacterID = @inbCharacterID OR IL.OwnerCharacterIDBefore = @inbCharacterID)
		      AND IL.LogDate >= @dt2StartLogDate AND IL.LogDate <= @dt2EndLogDate
			OPTION (MAXDOP 1);	
END TRY
BEGIN CATCH	
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO

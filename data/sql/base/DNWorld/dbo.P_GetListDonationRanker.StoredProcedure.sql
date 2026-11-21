USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDonationRanker]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-12-23
description : ?? ?? TOP 1 ?? ??? ?????.
result set : CharacterID/bigint, CharacterName/nvarchar, DonationAmount/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListDonationRanker]
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
	;WITH MD AS (
		SELECT TOP (1) MD.CharacterID, MD.DonationAmount
		FROM dbo.VIX_MonthlyDonation MD WITH (NOEXPAND, NOLOCK, INDEX(IX_NN_VIX_MonthlyDonation_DonationAmount_DonateDate))
		ORDER BY MD.DonationAmount DESC, DonateDate ASC
	)
	SELECT CH.CharacterID, CH.CharacterName, MD.DonationAmount
	FROM MD
		INNER LOOP JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON MD.CharacterID = CH.CharacterID;
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

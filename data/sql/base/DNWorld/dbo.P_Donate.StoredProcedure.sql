USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_Donate]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-12-23
description : ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_Donate]
	  @dtmDonateDate datetime --// ?? ?? (YYYY-MM-DD)
	, @inbCharacterID bigint --// ??? ID
	, @inbDonationAmount bigint --// ?? ??
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
	/**_# [DailyDonation] ???? UPSERT???.*/
	UPDATE dbo.DailyDonation
	SET DonationAmount += @inbDonationAmount
      , DonateDate = @dtmDonateDate
	FROM dbo.DailyDonation WITH (INDEX(PK_DailyDonation), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		INSERT dbo.DailyDonation (CharacterID, DonationAmount, DonateDate)
		VALUES (@inbCharacterID, @inbDonationAmount, @dtmDonateDate);
	END
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

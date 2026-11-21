USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMonitoringForCoin]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 14
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-22

description :
	???? ??? ?? (?? ?? + ?? ?? ??)
	???? - ??(??) / ??(?? ???? ??) / ?? / ????
result set : 
	Coin			bigint
	CharacterName	nvarchar(30)
	CharacterLevel	tinyint
	JobCode			tinyint
	CharacterID		bigint
	AccountID		int
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	10 = ?? ??
    11 = ???? ?? ???? ?? by ???
	12 = ?? ??? ??? ?? ?? by ???
**/

CREATE PROCEDURE [dbo].[P_GetListMonitoringForCoin]
	  @intWorldID	int				--// ??
	, @intRank		int		= null	--// ??
	, @inbCoin		bigint	= null	--// ????
	, @intMinCharacterLevel int =  null --// ?? ??? ??
	, @intMaxCharacterLevel int =  null --// ?? ??? ??
    , @bitExcludeRestraintAccount bit = NULL --// ???? ?? ?? (0=??, 1=???)
WITH EXECUTE AS 'SPExecutor'
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

	SET @intRank = ISNULL(@intRank, 0);
	SET @inbCoin = ISNULL(@inbCoin, 0);

	SET @intMinCharacterLevel = ISNULL(@intMinCharacterLevel, 0);
	SET @intMaxCharacterLevel = ISNULL(@intMaxCharacterLevel, 0);

	/**_# ?? ????? ??? 500?? ?????. */
	IF @intRank = 0 SET @intRank = 500;

	/**_# ?? ??? ????? ??? 1000?? ?????. */
	IF @intMaxCharacterLevel = 0 SET @intMaxCharacterLevel = 1000;

	/**_# ???? ???? ?????. */

	IF @bitExcludeRestraintAccount = 1 
	SELECT TOP (@intRank)
		   DC.Coin + DC.WarehouseCoin AS Coin
		 , DC.CharacterName
		 , DC.CharacterLevel
		 , DC.JobCode
		 , DC.CharacterID
		 , DC.AccountID
	  FROM dbo.DimCharacters DC WITH (NOLOCK) 
		   LEFT OUTER JOIN dbo.SS_RestraintedAccounts R ON DC.AccountID = R.AccountID
	 WHERE (DC.WorldID = @intWorldID)
	   AND (DC.Coin + DC.WarehouseCoin >= @inbCoin)
	   AND (DC.DeleteFlag = 0)
	   AND (DC.CharacterLevel >= @intMinCharacterLevel AND DC.CharacterLevel <= @intMaxCharacterLevel)
	   AND R.AccountID IS NULL
	 ORDER BY DC.Coin + DC.WarehouseCoin DESC;
	ELSE	
	SELECT TOP (@intRank)
		   DC.Coin + DC.WarehouseCoin AS Coin
		 , DC.CharacterName
		 , DC.CharacterLevel
		 , DC.JobCode
		 , DC.CharacterID
		 , DC.AccountID
	  FROM dbo.DimCharacters DC WITH (NOLOCK) 		   
	 WHERE (DC.WorldID = @intWorldID)
	   AND (DC.Coin + DC.WarehouseCoin >= @inbCoin)
	   AND (DC.DeleteFlag = 0)
	   AND (DC.CharacterLevel >= @intMinCharacterLevel AND DC.CharacterLevel <= @intMaxCharacterLevel)
	 ORDER BY DC.Coin + DC.WarehouseCoin DESC;

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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListGuildMember]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-08-13
description : ?? ?? ???? ?????.
result set : AccountID/int, AccountName/nvarchar, CharacterID/bigint, CharacterName/nvarchar, JobCode/tinyint, CharacterLevel/tinyint, LastLogoutDate/datetime2, GuildMemberRoleCode/tinyint, SelfIntroduction/nvarchar, TotalGuildPoint/int, UsedGuildPoint/int, RegisterDate/datetime2, DailyTakeItemCount/int, DailyTakeCoinAmount/int, LastGuildDepotUseDate/smalldatetime, MobileAuthenticationFlag/bit, LastVillageMapID/int, LastLogoutDate/datetime2, LastMapID/int, LastServerType/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103238 = ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetListGuildMember]
	  @intGuildID int = NULL --// ?? ID
	, @nvcGuildName nvarchar(50) = NULL --// ?? ??
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
	IF @intGuildID > 0
		SELECT CH.AccountID, CH.AccountName, CH.CharacterID, CH.CharacterName, CS.JobCode, CS.CharacterLevel, CS.LastLogoutDate
			, GM.GuildMemberRoleCode, GM.SelfIntroduction, GM.TotalGuildPoint, GM.UsedGuildPoint, GM.RegisterDate, GM.DailyTakeItemCount, GM.DailyTakeCoinAmount
			, GM.LastGuildDepotUseDate, ISNULL(CH.MobileAuthenticationFlag, 0) AS MobileAuthenticationFlag, CS.LastVillageMapID, CS.LastLogoutDate, CS.LastMapID, CS.LastServerType
		FROM dbo.GuildMembers GM WITH (NOLOCK)
			INNER JOIN dbo.Characters CH WITH (NOLOCK) ON GM.CharacterID = CH.CharacterID
			INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON CH.CharacterID = CS.CharacterID
		WHERE GuildID = @intGuildID;
	ELSE
		SELECT CH.AccountID, CH.AccountName, CH.CharacterID, CH.CharacterName, CS.JobCode, CS.CharacterLevel, CS.LastLogoutDate
			, GM.GuildMemberRoleCode, GM.SelfIntroduction, GM.TotalGuildPoint, GM.UsedGuildPoint, GM.RegisterDate, GM.DailyTakeItemCount, GM.DailyTakeCoinAmount
			, GM.LastGuildDepotUseDate, ISNULL(CH.MobileAuthenticationFlag, 0) AS MobileAuthenticationFlag, CS.LastVillageMapID, CS.LastLogoutDate, CS.LastMapID, CS.LastServerType
		FROM dbo.Guilds G WITH (NOLOCK)
			INNER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON G.GuildID = GM.GuildID
			INNER JOIN dbo.Characters CH WITH (NOLOCK) ON GM.CharacterID = CH.CharacterID
			INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON CH.CharacterID = CS.CharacterID
		WHERE G.GuildName = @nvcGuildName AND G.CloseFlag = CAST(0 AS bit);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103238;
		GOTO ErrorHandler;
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

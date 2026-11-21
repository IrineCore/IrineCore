USE [dnworld]
GO
/****** Object:  Table [dbo].[PCRoomCharacterLogoutLogs0]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PCRoomCharacterLogoutLogs0](
	[LogoutDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[LoginDate] [datetime2](0) NOT NULL,
	[LoginIP] [int] NOT NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_PCRoomCharacterLogoutLogs0_LogoutDate]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_PCRoomCharacterLogoutLogs0_LogoutDate] ON [dbo].[PCRoomCharacterLogoutLogs0]
(
	[LogoutDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO

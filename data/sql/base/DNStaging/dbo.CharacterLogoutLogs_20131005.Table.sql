USE [dnstaging]
GO
/****** Object:  Table [dbo].[CharacterLogoutLogs_20131005]    Script Date: 11/14/2026 9:00:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CharacterLogoutLogs_20131005](
	[LogoutDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[LoginDate] [datetime2](0) NOT NULL,
	[LoginIP] [int] NOT NULL,
	[SessionID] [int] NOT NULL,
	[PCRoomFlag] [bit] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_CharacterLogoutLogs_20131005_LogoutDate]    Script Date: 11/14/2026 9:00:06 AM ******/
CREATE CLUSTERED INDEX [IX_NC_CharacterLogoutLogs_20131005_LogoutDate] ON [dbo].[CharacterLogoutLogs_20131005]
(
	[LogoutDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_CharacterLogoutLogs_20131005_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_CharacterLogoutLogs_20131005_CharacterID] ON [dbo].[CharacterLogoutLogs_20131005]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_CharacterLogoutLogs_20131005_LoginIP]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_CharacterLogoutLogs_20131005_LoginIP] ON [dbo].[CharacterLogoutLogs_20131005]
(
	[LoginIP] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[CharacterLogoutLogs_20131005]  WITH CHECK ADD  CONSTRAINT [CK_CharacterLogoutLogs_20131005_LogoutDate] CHECK  (([LogoutDate]>='2013-10-05' AND [LogoutDate]<'2013-10-06'))
GO
ALTER TABLE [dbo].[CharacterLogoutLogs_20131005] CHECK CONSTRAINT [CK_CharacterLogoutLogs_20131005_LogoutDate]
GO

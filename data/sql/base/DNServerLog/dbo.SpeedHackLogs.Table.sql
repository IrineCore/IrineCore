USE [dnserverlog]
GO
/****** Object:  Table [dbo].[SpeedHackLogs]    Script Date: 11/14/2026 8:57:59 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SpeedHackLogs](
	[LogDate] [datetime2](0) NOT NULL,
	[WorldID] [int] NOT NULL,
	[AccountID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[CharacterName] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_SpeedHackLogs_LogDate]    Script Date: 11/14/2026 8:57:59 AM ******/
CREATE CLUSTERED INDEX [IX_NC_SpeedHackLogs_LogDate] ON [dbo].[SpeedHackLogs]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO

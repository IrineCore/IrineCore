USE [dnserverlog]
GO
/****** Object:  Table [dbo].[SpeedHackLogs20130905]    Script Date: 11/14/2026 8:57:59 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SpeedHackLogs20130905](
	[LogDate] [datetime2](0) NOT NULL,
	[WorldID] [int] NOT NULL,
	[AccountID] [int] NULL,
	[CharacterID] [bigint] NULL,
	[CharacterName] [nvarchar](60) COLLATE Chinese_PRC_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_SpeedHackLogs20130905_LogDate]    Script Date: 11/14/2026 8:57:59 AM ******/
CREATE CLUSTERED INDEX [IX_NC_SpeedHackLogs20130905_LogDate] ON [dbo].[SpeedHackLogs20130905]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[SpeedHackLogs20130905]  WITH CHECK ADD  CONSTRAINT [CK_SpeedHackLogs20130905_LogDate] CHECK  (([LogDate]>='2013-09-05' AND [LogDate]<'2013-09-06'))
GO
ALTER TABLE [dbo].[SpeedHackLogs20130905] CHECK CONSTRAINT [CK_SpeedHackLogs20130905_LogDate]
GO

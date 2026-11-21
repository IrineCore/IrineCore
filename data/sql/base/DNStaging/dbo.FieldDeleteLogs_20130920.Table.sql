USE [dnstaging]
GO
/****** Object:  Table [dbo].[FieldDeleteLogs_20130920]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FieldDeleteLogs_20130920](
	[LogDate] [datetime] NOT NULL,
	[FarmID] [int] NOT NULL,
	[FieldIndex] [smallint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[PlantDate] [datetime2](0) NOT NULL,
	[ElapsedTime] [int] NOT NULL,
	[HarvestFlag] [bit] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_FieldDeleteLogs_20130920_LogDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_FieldDeleteLogs_20130920_LogDate] ON [dbo].[FieldDeleteLogs_20130920]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[FieldDeleteLogs_20130920]  WITH CHECK ADD  CONSTRAINT [CK_FieldDeleteLogs_20130920_LogDate] CHECK  (([LogDate]>='2013-09-20' AND [LogDate]<'2013-09-21'))
GO
ALTER TABLE [dbo].[FieldDeleteLogs_20130920] CHECK CONSTRAINT [CK_FieldDeleteLogs_20130920_LogDate]
GO

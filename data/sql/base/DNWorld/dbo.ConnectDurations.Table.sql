USE [dnworld]
GO
/****** Object:  Table [dbo].[ConnectDurations]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ConnectDurations](
	[CharacterID] [bigint] NOT NULL,
	[ConnectDurationKey] [int] NOT NULL,
	[DurationTime] [bigint] NOT NULL,
	[CheckFlag] [bit] NOT NULL,
 CONSTRAINT [PK_ConnectDurations] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[ConnectDurationKey] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[ConnectDurations] ADD  CONSTRAINT [DF_ConnectDurations_CheckFlag]  DEFAULT ((0)) FOR [CheckFlag]
GO

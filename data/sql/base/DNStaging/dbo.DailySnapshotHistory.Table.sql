USE [dnstaging]
GO
/****** Object:  Table [dbo].[DailySnapshotHistory]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DailySnapshotHistory](
	[CheckDate] [datetime] NOT NULL,
	[Phase] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NOT NULL,
 CONSTRAINT [PK_DailySnapshotHistory] PRIMARY KEY CLUSTERED 
(
	[CheckDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, FILLFACTOR = 100) ON [PRIMARY]
) ON [PRIMARY]
GO
ALTER TABLE [dbo].[DailySnapshotHistory] ADD  CONSTRAINT [DF_DailySnapshotHistory_CheckDate]  DEFAULT (getdate()) FOR [CheckDate]
GO

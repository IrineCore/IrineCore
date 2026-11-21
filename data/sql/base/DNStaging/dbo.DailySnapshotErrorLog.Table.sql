USE [dnstaging]
GO
/****** Object:  Table [dbo].[DailySnapshotErrorLog]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DailySnapshotErrorLog](
	[ErrorDate] [datetime] NOT NULL,
	[ErrorStep] [nvarchar](256) COLLATE Chinese_PRC_CI_AS NULL,
	[ErrorNumber] [int] NULL,
	[ErrorMessage] [nvarchar](2048) COLLATE Chinese_PRC_CI_AS NULL,
 CONSTRAINT [PK_DailySnapshotErrorLog] PRIMARY KEY CLUSTERED 
(
	[ErrorDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO

USE [dnmembership]
GO
/****** Object:  Table [dbo].[SecondAuthLogsChangeTracking]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[SecondAuthLogsChangeTracking]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[SecondAuthLogsChangeTracking](
	[StatusCode] [tinyint] NOT NULL,
	[EndChangeDate] [datetime] NOT NULL,
	[RegistrationDate] [datetime] NOT NULL,
	[StartChangeDate] [datetime] NOT NULL,
	[StatusMessage] [nvarchar](2048) COLLATE Chinese_PRC_CI_AS NULL,
 CONSTRAINT [PK_SecondAuthLogsChangeTracking] PRIMARY KEY CLUSTERED 
(
	[StatusCode] ASC,
	[EndChangeDate] ASC,
	[RegistrationDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
